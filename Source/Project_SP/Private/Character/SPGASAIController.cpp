// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASCharacterBase.h"
#include "Tag/SPGameplayTags.h"
#include "TimerManager.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Game/ASPCombatGameMode.h"


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
			if (CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Mode_Battle))
			{
				OnBattleTagChanged(FSPGameplayTags::Get().State_Mode_Battle, 1);
			}
		}
	}

	// 필드 BT 실행
	if (FieldBT && (!CachedASC || !CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Mode_Battle)))
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
	UE_LOG(LogTemp, Warning, TEXT(">>> [AI] 몬스터 턴 시작! <<<"));

	TryExecuteAITurn();
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

void ASPGASAIController::TryExecuteAITurn()
{
	if (!CachedASC) return;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (CachedASC->HasMatchingGameplayTag(SPTags.State_Status_VisualPlaying))
	{
		GetWorld()->GetTimerManager().SetTimer(TurnWaitTimerHandle, this, &ASPGASAIController::TryExecuteAITurn, 0.1f, false);
		return;
	}

	if (CachedASC->HasMatchingGameplayTag(SPTags.State_Status_SkipTurn))
	{
		UE_LOG(LogTemp, Warning, TEXT(">>> [AI] 기절/혼절 상태입니다! 턴을 강제로 스킵합니다. <<<"));

		// 1턴만 쉬어야 하므로, 확인했으면 이름표(태그)를 바로 떼어줍니다!
		CachedASC->RemoveLooseGameplayTag(SPTags.State_Status_SkipTurn);

		// 행동 없이 즉시 턴 종료!
		FinishTurnDelayed();
		return; // 아래쪽에 있는 대본(스킬) 읽기 로직으로 못 넘어가게 원천 차단!
	}

	UE_LOG(LogTemp, Warning, TEXT(">>> [AI] 연출 종료 확인, 진짜 턴 행동 시작! <<<"));

	ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(GetPawn());
	if (!Monster || !Monster->MonsterDataAsset)
	{
		FGameplayTagContainer TagContainer(SPTags.Battle_Monster_BasicAttack);
		CachedASC->TryActivateAbilitiesByTag(TagContainer);
		return;
	}

	Monster->CurrentTurnCount++;

	float CurrentHP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute());
	float MaxHP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
	float HPPercent = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) * 100.0f : 100.0f;

	TSubclassOf<UGameplayAbility> BestAbility = nullptr;
	int32 HighestPriority = -1;

	for (const FMonsterAIPattern& Pattern : Monster->MonsterDataAsset->AIPatterns)
	{
		bool bConditionMet = false;

		switch (Pattern.Condition)
		{
		case EMonsterAICondition::Always:
			bConditionMet = true;
			break;

		case EMonsterAICondition::TurnCount:
			if (Pattern.ConditionValue > 0 && Monster->CurrentTurnCount % FMath::RoundToInt(Pattern.ConditionValue) == 0)
			{
				bConditionMet = true;
			}
			break;

		case EMonsterAICondition::HP_Below:
			if (HPPercent <= Pattern.ConditionValue)
			{
				bConditionMet = true;
			}
			break;

		case EMonsterAICondition::WasAttacked:
			if (Monster->bWasAttackedLastTurn)
			{
				bConditionMet = true;
			}
			break;

		case EMonsterAICondition::AllyCount_Below:
			if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
			{
				int32 ExpectedEnemyCount = 0;

				// 1. GameMode가 관리하는 현재 필드의 모든 적을 순회합니다.
				for (auto EnemyActor : GM->GetCurrentEnemies())
				{
					if (ASPGASCharacterBase* EnemyChar = Cast<ASPGASCharacterBase>(EnemyActor))
					{
						UAbilitySystemComponent* EnemyASC = EnemyChar->GetAbilitySystemComponent();
						if (EnemyASC)
						{
							// 기본적으로 필드에 살아있으므로 카운트 +1
							ExpectedEnemyCount++;

							// 🌟 [사전 예약 확인] 이 몬스터가 소환 스킬을 준비 중인가요?
							// 그렇다면 조만간 1마리가 더 태어날 것이므로 '예상 카운트'에 미리 +1을 더해줍니다!
							if (EnemyASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Monster.CastingHowling"))))
							{
								ExpectedEnemyCount++;
							}
						}
					}
				}

				// 2. 최종 합산된 '예상 적 숫자'가 조건값 이하일 때만 True 반환!
				if (ExpectedEnemyCount <= FMath::RoundToInt(Pattern.ConditionValue))
				{
					bConditionMet = true;
				}
			}
			break;

		case EMonsterAICondition::HasGameplayTag:
			if (CachedASC->HasMatchingGameplayTag(Pattern.RequiredTag))
			{
				bConditionMet = true;
			}
			break;
		}

		// 조건이 맞았고, 지금까지 찾은 스킬보다 우선순위가 더 높다면 교체!
		if (bConditionMet && Pattern.Priority > HighestPriority)
		{
			if (Pattern.AbilityToExecute)
			{
				// 🌟 [추가된 핵심 로직] GAS의 권능을 이용해 쿨타임/코스트 검사를 먼저 합니다!
				bool bCanActivate = false;

				// 마녀가 이 스킬(GA)을 가지고 있는지 스펙(Spec)을 찾아옵니다.
				FGameplayAbilitySpec* Spec = CachedASC->FindAbilitySpecFromClass(Pattern.AbilityToExecute);

				if (Spec && Spec->Ability)
				{
					// CanActivateAbility: "지금 쿨타임 안 돌고 있니? 마나(코스트)는 충분하니?" 를 물어봅니다.
					bCanActivate = Spec->Ability->CanActivateAbility(Spec->Handle, CachedASC->AbilityActorInfo.Get());
				}

				// 쿨타임이 아니라서 당장 쓸 수 있을 때만 '최고의 스킬'로 낙점!
				if (bCanActivate)
				{
					HighestPriority = Pattern.Priority;
					BestAbility = Pattern.AbilityToExecute;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[AI] %s 패턴은 조건이 맞았으나, 쿨타임 중이라 이번 턴엔 보류합니다."), *Pattern.AbilityToExecute->GetName());
				}
			}
		}
	}

	// 5. 찾아낸 최고의 스킬(BestAbility) 발동!
	if (BestAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI] 패턴 조건 달성! 스킬 실행: %s"), *BestAbility->GetName());
		CachedASC->TryActivateAbilityByClass(BestAbility);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI] 만족하는 패턴이 없어 기본 공격을 실행합니다."));
		FGameplayTagContainer TagContainer(SPTags.Battle_Monster_BasicAttack);
		CachedASC->TryActivateAbilitiesByTag(TagContainer);
	}

	// 6. 다음 턴을 위해 "저번 턴에 맞았음" 기억 리셋!
	Monster->SetAttackedLastTurn(false);
}
