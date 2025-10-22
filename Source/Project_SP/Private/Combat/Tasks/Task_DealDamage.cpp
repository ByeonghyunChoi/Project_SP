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
        UAttributesComponent* InstigatorStats = Instigator->GetAttributesComponent(); // ∞°«ÿ¿⁄ Ω∫≈»

        for (ACombatPawn* Target : Targets)
        {
            if (Target && Target->GetAttributesComponent() && Target->GetGameEventComponent())
            {
                UAttributesComponent* TargetStats = Target->GetAttributesComponent(); // «««ÿ¿⁄ Ω∫≈»

                float FinalDamage = UCombatStatics::CalculateDamage(InstigatorStats, TargetStats, SkillCoefficient);
                EDamageFloaterType DamageType = EDamageFloaterType::Normal;

                if (FMath::FRand() < InstigatorStats->GetCurrentStats().fCriticalChance)
                {
                    FinalDamage *= InstigatorStats->GetCurrentStats().fCriticalDamageMultiplier;
                    DamageType = EDamageFloaterType::Critical;
                }

                const float RoundedDamage = FMath::RoundToFloat(FinalDamage);

                Target->GetAttributesComponent()->ApplyHealthChange(-RoundedDamage, Instigator);

                Target->GetGameEventComponent()->BroadcastDamageFinalized(Target, RoundedDamage, DamageType, Instigator);
            }
        }
    }
    FinishTask();
}

