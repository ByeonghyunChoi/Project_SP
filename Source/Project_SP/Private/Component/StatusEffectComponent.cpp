#include "Component/StatusEffectComponent.h"
#include "Character/CombatPawn.h"
#include "Component/AttributesComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/CombatStatics.h"
#include "Component/GameEventComponent.h"

UStatusEffectComponent::UStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UStatusEffectComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerPawn = Cast<ACombatPawn>(GetOwner());
    if (OwnerPawn)
    {
        OwnerAttributesComp = OwnerPawn->GetAttributesComponent();
    }
}

void UStatusEffectComponent::ApplyStatusEffect(FName StatusEffectID, ACombatPawn* Instigator)
{
    if (!StatusEffectDataTable || !OwnerPawn || !Instigator) return;

    FStatusEffectData* EffectData = StatusEffectDataTable->FindRow<FStatusEffectData>(StatusEffectID, TEXT("Find Status Effect Data"));
    if (!EffectData) return;

    // 조합 로직
    if (EffectData->CombinationRules.Num() > 0)
    {
        for (const TPair<FName, FName>& Rule : EffectData->CombinationRules)
        {
            const FName& TargetID = Rule.Key;
            const FName& ResultID = Rule.Value;

            int32 FoundIndex = ActiveStatusEffects.IndexOfByPredicate([&](const FActiveStatusEffect& Effect) {
                return Effect.EffectID == TargetID;
                });

            if (FoundIndex != INDEX_NONE)
            {
                UE_LOG(LogTemp, Warning, TEXT("Combination Occured! Removing %s and applying %s"), *TargetID.ToString(), *ResultID.ToString());
                ActiveStatusEffects.RemoveAt(FoundIndex);
                ApplyStatusEffect(ResultID, Instigator);
                return;
            }
        }
    }

    // 일반 효과 적용
    FActiveStatusEffect NewEffect;
    NewEffect.EffectID = StatusEffectID;
    NewEffect.RemainingTurns = EffectData->TurnDuration;
    NewEffect.Instigator = Instigator;
    ActiveStatusEffects.Add(NewEffect);
    UE_LOG(LogTemp, Log, TEXT("%s is now affected by %s"), *OwnerPawn->GetName(), *EffectData->DisplayName.ToString());

    // 스탯 변경 효과가 있다면 즉시 재계산 및 적용
    RecalculateStatModifiers();
}

void UStatusEffectComponent::OnTurnStarted()
{
    if (!OwnerPawn || !StatusEffectDataTable) return;

    TArray<FActiveStatusEffect> EffectsToRemove;

    // 턴 시작 시 효과 발동
    for (FActiveStatusEffect& ActiveEffect : ActiveStatusEffects)
    {
        FStatusEffectData* EffectData = StatusEffectDataTable->FindRow<FStatusEffectData>(ActiveEffect.EffectID, TEXT(""));
        if (EffectData && ActiveEffect.Instigator.IsValid())
        {
            for (const FStatusSubEffect& SubEffect : EffectData->SubEffects)
            {
                if (SubEffect.EffectType == EStatusEffectType::DamageOverTime)
                {
                    float FinalDamage = UCombatStatics::CalculateStatusEffectDamage(
                        ActiveEffect.Instigator.Get(),
                        OwnerPawn,
                        SubEffect
                    );

                    ACombatPawn* InstigatorPawn = ActiveEffect.Instigator.Get();
                    if (!InstigatorPawn) continue;

                    if (OwnerAttributesComp)
                    {
                        OwnerAttributesComp->ApplyHealthChange(-FinalDamage, InstigatorPawn);
                    }

                    // 2. 새 이벤트(OnDamageFinalized)로 방송합니다.
                    if (OwnerPawn->GetGameEventComponent())
                    {
                        OwnerPawn->GetGameEventComponent()->BroadcastDamageFinalized(OwnerPawn, FinalDamage, EDamageFloaterType::StatusEffect, InstigatorPawn);
                    }

                    UE_LOG(LogTemp, Log, TEXT("상태 이상 데미지: %f"), FinalDamage);

                }
                // '혼절' 같은 행동 불가 로직도 여기서 처리 가능
            }
        }
    }

    // 턴 감소 및 만료된 효과 제거 준비
    for (FActiveStatusEffect& ActiveEffect : ActiveStatusEffects)
    {
        ActiveEffect.RemainingTurns--;
        if (ActiveEffect.RemainingTurns <= 0)
        {
            EffectsToRemove.Add(ActiveEffect);
        }
    }

    // 만료된 효과 실제 제거
    for (const FActiveStatusEffect& EffectToRemove : EffectsToRemove)
    {
        ActiveStatusEffects.RemoveAll([&](const FActiveStatusEffect& Effect) {
            return Effect.EffectID == EffectToRemove.EffectID;
            });
        UE_LOG(LogTemp, Log, TEXT("%s's %s has worn off."), *OwnerPawn->GetName(), *EffectToRemove.EffectID.ToString());
    }

    // 효과가 제거되었으므로 스탯 재계산
    if (EffectsToRemove.Num() > 0)
    {
        RecalculateStatModifiers();
    }
}

void UStatusEffectComponent::RecalculateStatModifiers()
{
    if (!OwnerAttributesComp) return;

    // 2. 현재 걸려있는 모든 효과를 순회하며 스탯 변경
    for (const FActiveStatusEffect& ActiveEffect : ActiveStatusEffects)
    {
        FStatusEffectData* EffectData = StatusEffectDataTable->FindRow<FStatusEffectData>(ActiveEffect.EffectID, TEXT(""));
        if (EffectData)
        {
            for (const FStatusSubEffect& SubEffect : EffectData->SubEffects)
            {
                if (SubEffect.EffectType == EStatusEffectType::Stat_Debuff)
                {
                    if (SubEffect.StatToModify == EStatToModify::DefensePower)
                    {
                        float OriginalDefense = OwnerAttributesComp->GetCurrentStats().fDefensePower;
                    }
                }
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("--- RecalculateStatModifiers END ---"));
}