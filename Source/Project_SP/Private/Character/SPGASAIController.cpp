// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASCharacterBase.h"
#include "Tag/SPGameplayTags.h"
#include "TimerManager.h"


ASPGASAIController::ASPGASAIController()
{
}


void ASPGASAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


	ASPGASMonsterCharacter* MonsterCharacter = Cast< ASPGASMonsterCharacter>(InPawn);
	if (MonsterCharacter)
	{
		CachedASC = MonsterCharacter->GetAbilitySystemComponent();
		if (CachedASC)
		{
			// 전투 태그 상태 변화 감시 
			CachedASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_Mode_Battle, EGameplayTagEventType::AnyCountChange)
				.AddUObject(this, &ASPGASAIController::OnBattleTagChanged);

			CachedASC->GenericGameplayEventCallbacks
				.FindOrAdd(FSPGameplayTags::Get().Event_Battle_TurnStart)
				.AddUObject(this, &ASPGASAIController::OnTurnStartEvent);
		}
	}

	// 필드 BT 실행
	if (FieldBT)
	{
		RunBehaviorTree(FieldBT);
	}
}

void ASPGASAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ASPGASAIController::OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (!BTComp) return;

	if (NewCount > 0) // [전투 모드 진입]
	{
		// 실시간 이동 명령 즉시 중단
		StopMovement();

		// 비헤이비어 트리 일시 정지 (모든 자율 판단 중지)
		BTComp->StopLogic("Entered Battle Mode");

		UE_LOG(LogTemp, Warning, TEXT("AI: 전투 모드 진입 - 자율 로직 정지"));
	}
	else // [필드 모드 복귀]
	{
		// 비헤이비어 트리 재개
		if (FieldBT)
		{
			RunBehaviorTree(FieldBT);
		}

		UE_LOG(LogTemp, Warning, TEXT("AI: 필드 모드 복귀 - 자율 로직 재개"));
	}
}

void ASPGASAIController::OnTurnStartEvent(const FGameplayEventData* Payload)
{
	// 1. 로그 출력
	UE_LOG(LogTemp, Warning, TEXT(">>> [AI] 몬스터 턴 시작! (1초 뒤 종료) <<<"));

	if (CachedASC)
	{
		// 🌟 몬스터의 기본 공격 스킬 태그를 찾아서 실행하라고 지시합니다.
		// (프로젝트 태그에 맞게 수정하세요. 예: Ability.Monster.BasicAttack)
		FGameplayTag AttackTag = FSPGameplayTags::Get().Battle_Monster_BasicAttack;
		FGameplayTagContainer TagContainer(AttackTag);

		bool bSuccess = CachedASC->TryActivateAbilitiesByTag(TagContainer);
	}
}

// [추가] 실제로 턴을 넘기는 함수
void ASPGASAIController::FinishTurnDelayed()
{
	if (ASPGASCharacterBase* GASCharacter = Cast<ASPGASCharacterBase>(GetPawn()))
	{
		// GameMode에게 턴 종료 알림
		GASCharacter->FinishTurn();
	}
}