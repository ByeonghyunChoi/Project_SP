// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_DealDamageToAll.h"
#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"

void UTask_DealDamageToAll::ExecuteTask_Implementation()
{
	// 1. 필수 객체 확인
	if (!BattleManager || !Instigator)
	{
		FinishTask();
		return;
	}

	// 2. 적대 세력 판별 (내가 플레이어면 적은 Enemy, 내가 적이면 적은 Player)
	EFaction MyFaction = Instigator->GetFaction();
	EFaction TargetFaction = (MyFaction == EFaction::Player) ? EFaction::Enemy : EFaction::Player;

	// 3. BattleManager에서 모든 전투원을 가져와 적대 세력만 필터링
	TArray<ACombatPawn*> AllEnemies;
	for (ACombatPawn* Pawn : BattleManager->GetAllCombatants())
	{
		if (Pawn && Pawn->GetFaction() == TargetFaction &&
			Pawn->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			AllEnemies.Add(Pawn);
		}
	}

	// 4. Targets 배열을 잠시 '모든 적' 목록으로 교체
	// (UTask_DealDamage는 Targets 배열을 순회하며 데미지를 주기 때문)
	TArray<ACombatPawn*> OriginalTargets = Targets;
	Targets = AllEnemies;

	// 5. 부모 클래스의 로직 실행 (데미지 계산 및 적용)
	Super::ExecuteTask_Implementation();

	// 6. (선택사항) Targets 원상 복구
	Targets = OriginalTargets;

}
