// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_DealDamage.h"
#include "Character/CombatPawn.h"
#include "Component/AttributesComponent.h"
#include "Combat/GameAction.h" 
#include "Combat/CombatStatics.h"
#include "Component/GameEventComponent.h"

void UTask_DealDamage::ExecuteTask_Implementation()
{
    UGameAction* OwningAction = Cast<UGameAction>(GetOuter());
    if (Instigator && OwningAction && Instigator->GetAttributesComponent() && Targets.Num() > 0)
    {
        const float SkillCoefficient = OwningAction->GetData().SkillCoefficient;
        UAttributesComponent* InstigatorStats = Instigator->GetAttributesComponent(); // 가해자 스탯

        for (ACombatPawn* Target : Targets)
        {
            if (Target && Target->GetAttributesComponent() && Target->GetGameEventComponent())
            {
                UAttributesComponent* TargetStats = Target->GetAttributesComponent(); // 피해자 스탯

                // --- [로직 변경됨] ---

                // 1. 치명타가 *제외된* 기본 데미지를 계산합니다.
                // (UCombatStatics::CalculateDamage에서 치명타 로직을 제거해야 함 - 4단계 참고)
                float FinalDamage = UCombatStatics::CalculateDamage(InstigatorStats, TargetStats, SkillCoefficient);
                EDamageFloaterType DamageType = EDamageFloaterType::Normal;

                // 2. 치명타 계산 (CombatStatics [cite: 186]에서 로직 이동)
                if (FMath::FRand() < InstigatorStats->GetCurrentStats().fCriticalChance)
                {
                    FinalDamage *= InstigatorStats->GetCurrentStats().fCriticalDamageMultiplier;
                    DamageType = EDamageFloaterType::Critical;
                }

                // 3. 데미지를 직접 적용합니다. (ApplyDamage 대신)
                Target->GetAttributesComponent()->ApplyHealthChange(-FinalDamage, Instigator);

                // 4. 새 이벤트(OnDamageFinalized)로 데미지 정보를 방송합니다.
                Target->GetGameEventComponent()->BroadcastDamageFinalized(Target, FinalDamage, DamageType, Instigator);
            }
        }
    }
    FinishTask();
}

