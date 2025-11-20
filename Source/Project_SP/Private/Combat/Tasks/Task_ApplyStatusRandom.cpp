// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_ApplyStatusRandom.h"
#include "Component/StatusEffectComponent.h"
#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"

void UTask_ApplyStatusRandom::ExecuteTask_Implementation()
{
	if (!BattleManager || !Instigator)
	{
		FinishTask();
		return;
	}

	// 상태 이상을 걸 대상을 모아둘 집합 (중복 방지를 위해 Set 사용)
	TSet<ACombatPawn*> PawnsToApply;

	// 1. 현재 타겟 추가 (기본 공격 대상)
	if (Targets.IsValidIndex(0) && Targets[0])
	{
		PawnsToApply.Add(Targets[0]);
	}

	// 2. 적대 세력 판별
	EFaction MyFaction = Instigator->GetFaction();
	EFaction TargetFaction = (MyFaction == EFaction::Player) ? EFaction::Enemy : EFaction::Player;

	// 3. 랜덤 타겟 후보군 생성 (죽지 않았고, 현재 타겟이 아닌 적)
	TArray<ACombatPawn*> PotentialRandomTargets;
	for (ACombatPawn* Pawn : BattleManager->GetAllCombatants())
	{
		if (Pawn && Pawn->GetFaction() == TargetFaction &&
			Pawn->GetCombatPawnState() != ECombatPawnState::Defeated &&
			!PawnsToApply.Contains(Pawn)) // 이미 타겟인 대상은 제외
		{
			PotentialRandomTargets.Add(Pawn);
		}
	}

	// 4. 후보가 있다면 랜덤으로 1명 뽑아서 추가
	if (PotentialRandomTargets.Num() > 0)
	{
		int32 RandIndex = FMath::RandRange(0, PotentialRandomTargets.Num() - 1);
		PawnsToApply.Add(PotentialRandomTargets[RandIndex]);
	}

	// 5. 최종 선별된 대상들에게 상태 이상 적용
	for (ACombatPawn* TargetPawn : PawnsToApply)
	{
		if (UStatusEffectComponent* StatusComp = TargetPawn->GetStatusEffectComponent())
		{
			StatusComp->ApplyStatusEffect(StatusEffectID, Instigator);
		}
	}

	FinishTask();
}
