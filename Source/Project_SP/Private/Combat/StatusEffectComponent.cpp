// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/StatusEffectComponent.h"
#include "Combat/CombatPawn.h"
#include "Combat/CharacterStatsComponent.h"
#include "Kismet/GameplayStatics.h"

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
        OwnerStatsComp = OwnerPawn->GetStatsComponent();
    }
}

void UStatusEffectComponent::ApplyStatusEffect(FName StatusEffectID, ACombatPawn* Instigator)
{
    if (!StatusEffectDataTable || !OwnerPawn || !Instigator) return;

    FStatusEffectData* EffectData = StatusEffectDataTable->FindRow<FStatusEffectData>(StatusEffectID, TEXT("Find Status Effect Data"));
    if (!EffectData) return;

    // 조합 로직
    if (EffectData->CombinationTargetID != NAME_None)
    {
        int32 FoundIndex = ActiveStatusEffects.IndexOfByPredicate([&](const FActiveStatusEffect& Effect) {
            return Effect.EffectID == EffectData->CombinationTargetID;
            });

        if (FoundIndex != INDEX_NONE)
        {
            UE_LOG(LogTemp, Warning, TEXT("Combination Occured! Removing %s and applying %s"), *EffectData->CombinationTargetID.ToString(), *EffectData->CombinationResultID.ToString());
            ActiveStatusEffects.RemoveAt(FoundIndex);
            // 재귀 호출로 조합된 새로운 효과를 적용
            ApplyStatusEffect(EffectData->CombinationResultID, Instigator);
            return;
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
                    float Damage = ActiveEffect.Instigator->GetStatsComponent()->GetAttackPower() * SubEffect.EffectMagnitude;
                    UGameplayStatics::ApplyDamage(OwnerPawn, Damage, ActiveEffect.Instigator->GetController(), ActiveEffect.Instigator.Get(), UDamageType::StaticClass());
                    UE_LOG(LogTemp, Warning, TEXT("%s takes %f damage from %s"), *OwnerPawn->GetName(), Damage, *EffectData->DisplayName.ToString());
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
    if (!OwnerStatsComp) return;

    // 1. 데이터 테이블에서 원본 스탯을 다시 불러와 초기화
    OwnerStatsComp->InitializeStatsFromDataTable();

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
                        float OriginalDefense = OwnerStatsComp->GetDefensePower();
                        OwnerStatsComp->SetDefensePower(OriginalDefense * (1.0f + SubEffect.EffectMagnitude)); // Magnitude는 -0.25와 같은 음수값
                    }
                }
            }
        }
    }
}

