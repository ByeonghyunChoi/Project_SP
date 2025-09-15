#include "Combat/GameAction.h"
#include "Combat/CombatPawn.h"
#include "Combat/AttributesComponent.h"
#include "Combat/StatusEffectComponent.h" 
#include "Core/BattleManager.h"
#include "Kismet/GameplayStatics.h" 
#include "Data/ActionData.h"
#include "Combat/CombatStatics.h"

UGameAction::UGameAction()
{
}


void UGameAction::ExecuteAction(ACombatPawn* Instigator, const FActionData& ActionData, ABattleManager* BattleManagerRef, ACombatPawn* TargetPawn, const TArray<ACombatPawn*>& TargetPawns)
{
    if (Instigator && Instigator->GetAttributesComponent() && ActionData.CostType != ECostType::None)
    {
        if (ActionData.CostType == ECostType::SP && Instigator->GetAttributesComponent()->GetSkillPoint() >= ActionData.CostAmount)
        {
            Instigator->GetAttributesComponent()->ApplySPChange(Instigator->GetAttributesComponent()->GetSkillPoint() - ActionData.CostAmount);
        }
    }

    if (ActionData.ActionType == EActionType::Attack)
    {
        if (Instigator && Instigator->GetAttributesComponent())
        {
            // ModifySP 함수는 최대 SP를 넘지 않도록 자동으로 값을 조절해줍니다.
            Instigator->GetAttributesComponent()->ApplySPChange(Instigator->GetAttributesComponent()->GetSkillPoint() + 1);
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
                if (Combatant && Combatant->GetAttributesComponent() && Combatant->GetAttributesComponent()->GetCurrentStats().fCurrentHealth > 0 &&
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


    // ----- 최종 타겟에 효과 적용 -----
    for (ACombatPawn* CurrentTarget : FinalTargets)
    {
        if (CurrentTarget && Instigator)
        {
            //---- 데미지 계산
            UAttributesComponent* AttackerStats = Instigator->GetAttributesComponent();
            UAttributesComponent* TargetStats = CurrentTarget->GetAttributesComponent();
            if (AttackerStats && TargetStats)
            {
                for (int32 i = 0; i < ActionData.NumberOfHits; ++i)
                {
                    float FinalDamage = UCombatStatics::CalculateDamage(
                        Instigator->GetAttributesComponent(),
                        CurrentTarget->GetAttributesComponent(),
                        ActionData.SkillCoefficient
                    );

                    UE_LOG(LogTemp, Log, TEXT("Final Calculated Damage: %f"), FinalDamage);
                    // UGameplayStatics::ApplyDamage를 호출하여 TakeDamage 함수를 통해 데미지 적용
                    UGameplayStatics::ApplyDamage(CurrentTarget, FinalDamage, Instigator ? Instigator->GetController() : nullptr, Instigator, UDamageType::StaticClass());
                }
                if (ActionData.StatusEffectIDToApply != NAME_None && FMath::FRand() < ActionData.StatusEffectChance)
                {
                    if (CurrentTarget->GetStatusEffectComponent())
                    {
                        CurrentTarget->GetStatusEffectComponent()->ApplyStatusEffect(ActionData.StatusEffectIDToApply, Instigator);
                    }
                }
            }
        }
    }
}

bool UGameAction::HasEnoughCost(ACombatPawn* Instigator, const FActionData& ActionData) const
{
    if (ActionData.CostType == ECostType::None)
        return true;

    if (!Instigator || !Instigator->GetAttributesComponent())
        return false;

    UAttributesComponent* StatsComp = Instigator->GetAttributesComponent();

    if (ActionData.CostType == ECostType::SP)
    {
        return StatsComp->GetSkillPoint() >= ActionData.CostAmount;
    }
    return false;
}