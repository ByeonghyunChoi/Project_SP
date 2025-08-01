// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/GameAction.h"
#include "Combat/CombatPawn.h"
#include "Combat/CharacterStatsComponent.h"
#include "Core/BattleManager.h"
#include "Kismet/GameplayStatics.h" 
#include "Data/ActionData.h"

UGameAction::UGameAction()
{
}


void UGameAction::ExecuteAction(ACombatPawn* Instigator, const FActionData& ActionData, ABattleManager* BattleManagerRef, ACombatPawn* TargetPawn, const TArray<ACombatPawn*>& TargetPawns)
{
	if (Instigator && Instigator->GetStatsComponent() && ActionData.CostType != ECostType::None)
	{
		if (ActionData.CostType == ECostType::SP && Instigator->GetStatsComponent()->fCurrentSP >= ActionData.CostAmount)
		{ 
			Instigator->GetStatsComponent()->SetCurrentSP(Instigator->GetStatsComponent()->GetCurrentSP() - ActionData.CostAmount);
		}
	}

	TArray<ACombatPawn*> FinalTargets;

    if (BattleManagerRef) // BattleManagerRef를 통해 모든 전투원 목록에 접근
    {
        if (ActionData.TargetingType == ETargetingType::Single)
        {
            // TargetPawns (UI에서 확정된 타겟) 사용 또는 TargetPawn (AI에서 결정한 타겟) 사용
            if (TargetPawns.Num() > 0 && TargetPawns[0]) FinalTargets.Add(TargetPawns[0]);
            else if (TargetPawn) FinalTargets.Add(TargetPawn);
        }
        else if (ActionData.TargetingType == ETargetingType::All)
        {
            for (ACombatPawn* Combatant : BattleManagerRef->AllCombatants)
            {
                if (Combatant && Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0 &&
                    Combatant->GetFaction() != Instigator->GetFaction())
                {
                    FinalTargets.Add(Combatant);
                }
            }
        }
        else if (ActionData.TargetingType == ETargetingType::Dual) // 2인 타겟팅
        {
            // 여기서는 UI에서 확정된 ConfirmedTargetPawns를 그대로 FinalTargets로 사용
            FinalTargets = TargetPawns;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[UGameAction] BattleManagerRef is null. Cannot determine targets for %s."), *ActionData.DisplayName.ToString());
        return; // BattleManager 없으면 타겟 결정 불가
    }

    float FinalDamage = 0.0f;

    // ----- 최종 타겟에 효과 적용 -----
    for (ACombatPawn* CurrentTarget : FinalTargets)
    {
        if (CurrentTarget &&  Instigator)
        {
            //---- 데미지 계산
            UCharacterStatsComponent* AttackerStats = Instigator->GetStatsComponent();
            UCharacterStatsComponent* TargetStats = CurrentTarget->GetStatsComponent();
            if (AttackerStats && TargetStats)
            {
                float BaseDamage = AttackerStats->GetAttackPower() * ActionData.SkillCoefficient;
                float DamageMultiCoef = (1 + AttackerStats->GetDamageIncreaseMultiplier() - AttackerStats->GetDamageReductionMultiplier());
                float DefendCoef = 1 - (TargetStats->GetDefensePower() / (TargetStats->GetDefensePower() + 500)) + AttackerStats->GetArmorPenetration();
                float HitChance = 1 - (TargetStats->GetEvasion() - AttackerStats->GetHitProbability());
                if (FMath::FRand() > HitChance)
                {
                    BaseDamage *= 0.5;
                }
                if (FMath::FRand() < AttackerStats->GetCriticalChance())
                {
                    BaseDamage *= AttackerStats->GetCriticalDamageMultiplier();
                }
                //레벨 계수 추가해야 함(모르고 빼먹음;;;)

                FinalDamage = BaseDamage * DamageMultiCoef * DefendCoef; //레벨 계수도 곱해야 함
            }
            UE_LOG(LogTemp, Log, TEXT("Final Calculated Damage: %f"), FinalDamage);
            // UGameplayStatics::ApplyDamage를 호출하여 TakeDamage 함수를 통해 데미지 적용
            UGameplayStatics::ApplyDamage(CurrentTarget, FinalDamage, Instigator ? Instigator->GetController() : nullptr, Instigator, UDamageType::StaticClass());

        }
    }
}

bool UGameAction::HasEnoughCost(ACombatPawn* Instigator, const FActionData& ActionData) const
{
	if (ActionData.CostType == ECostType::None)
		return true;

	if (!Instigator || !Instigator->GetStatsComponent())
		return false;

	UCharacterStatsComponent* StatsComp = Instigator->GetStatsComponent();

	if (ActionData.CostType == ECostType::SP)
	{
		return StatsComp->GetCurrentSP() >= ActionData.CostAmount;
	}
	return false;
}
