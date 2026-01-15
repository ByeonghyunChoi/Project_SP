// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "Character/SPGASCharacterBase.h"
#include "Character/SPGASMonsterState.h"

ASPGASAIController::ASPGASAIController()
{
	bWantsPlayerState = true;
}


void ASPGASAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 생성된 MonsterState로부터 ASC를 가져옴
	if (ASPGASMonsterState* MS = GetPlayerState<ASPGASMonsterState>())
	{
		CachedASC = MS->GetAbilitySystemComponent();
		if (CachedASC)
		{
			// 전투 태그 상태 변화 감시 
			CachedASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_Mode_Battle, EGameplayTagEventType::AnyCountChange)
				.AddUObject(this, &ASPGASAIController::OnBattleTagChanged);
		}
	}

	// 필드 BT 실행
	if (FieldBT)
	{
		RunBehaviorTree(FieldBT);
	}
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