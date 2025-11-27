// Core/BattleManager.cpp

#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"
#include "Character/PlayerCharacter.h"
#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"
#include "Component/GameEventComponent.h"
#include "Component/CombatCameraComponent.h"
#include "Component/TurnSchedulerComponent.h"
#include "Combat/CombatTask.h"
#include "Combat/Tasks/Task_WaitForAnimNotify.h"
#include "Combat/Tasks/Task_ExecuteParrySwitch.h" 
#include "Combat/Tasks/Task_MoveCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Character/MyPlayerController.h"
#include "TimerManager.h"
#include "Combat/BattleTransitionManagerSubsystem.h"
#include "Component/PlayerCombatControlComponent.h"
#include "Component/StatusEffectComponent.h"
#include "SubSystem/TimeForceSubsystem.h"
#include "Component/ActionComponent.h"
#include "Combat/GameAction.h"

ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CameraComponent = CreateDefaultSubobject<UCombatCameraComponent>(TEXT("CameraComponent"));
	TurnScheduler = CreateDefaultSubobject<UTurnSchedulerComponent>(TEXT("TurnSchedulerComponent"));
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentBattleState != EBattleState::InProgress) return;

	// 1. 현재 실행 중인 태스크가 있다면 틱 공급
	if (bIsProcessingTask && CurrentTask)
	{
		if (CurrentTask->IsLatent())
		{
			CurrentTask->TickTask(DeltaTime);
		}
		return; // 태스크 실행 중에는 아래 로직 수행 안 함
	}

	// 2. 대기 중인 태스크가 있다면 다음 태스크 실행
	if (TaskQueue.Num() > 0)
	{
		ProcessTaskQueue();
		return;
	}

	// --- [여기 도달했다는 것은 실행/대기 중인 태스크가 없다는 뜻] ---

	// 3. [추가] 태스크가 다 끝났고, 턴 시작을 대기 중이었다면 턴 시작 시퀀스 진입
	if (bWaitingForTasksToStartTurn)
	{
		bWaitingForTasksToStartTurn = false;
		OnTurnStartSequenceFinished();
		return;
	}

	// 4. 턴 스택이 비었다면 다음 턴 결정
	if (TurnStack.IsEmpty())
	{
		DecideAndStartNextTurn();
	}
}

void ABattleManager::StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
	CachedPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerControlComponents.Empty();

	AllCombatants.Empty();
	AllCombatants.Append(PlayerParty);
	AllCombatants.Append(EnemyParty);

	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant)
		{
			if (Combatant->GetFaction() == EFaction::Player) // 플레이어 컴포넌트 저장
			{
				if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Combatant))
				{
					UPlayerCombatControlComponent* ControlComp = PlayerChar->FindComponentByClass<UPlayerCombatControlComponent>();
					if (ControlComp)
					{
						PlayerControlComponents.Add(ControlComp);
					}
				}
			}

			if (Combatant->GetGameEventComponent())
			{
				if (Combatant->GetFaction() == EFaction::Enemy) // 적 패링 이벤트 바인딩
				{
					Combatant->GetGameEventComponent()->OnParryWindowOpened.AddDynamic(this, &ABattleManager::HandleEnemyParryWindowOpened);
					Combatant->GetGameEventComponent()->OnParryWindowClosed.AddDynamic(this, &ABattleManager::HandleEnemyParryWindowClosed);
				}
				Combatant->GetGameEventComponent()->OnActionExecutionFinished.AddDynamic(this, &ABattleManager::HandleActionFinished);
				Combatant->GetGameEventComponent()->OnInterruptRequest.AddDynamic(this, &ABattleManager::HandleInterruptRequest);
				Combatant->GetGameEventComponent()->OnDamageFinalized.AddDynamic(this, &ABattleManager::HandleDamageReceived);
			}
			if (Combatant->GetAttributesComponent())
			{
				Combatant->GetAttributesComponent()->OnHealthDepleted.AddDynamic(this, &ABattleManager::HandleCombatantDied);
			}
		}
	}
	CurrentBattleState = EBattleState::InProgress;
	SetActorTickEnabled(true);

	DecideAndStartNextTurn();
}

void ABattleManager::EndBattle()
{
	CurrentBattleState = EBattleState::Ended;
	TurnStack.Empty();
	SetActorTickEnabled(false);

	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->GetActionComponent())
		{
			Combatant->GetActionComponent()->ResetActiveAction();
		}
	}

	AllCombatants.Empty();
	PlayerControlComponents.Empty();
	ClearTaskQueue();

	GetWorldTimerManager().SetTimer(
		BattleEndTimerHandle,
		this,
		&ABattleManager::TriggerFieldTransition,
		BattleEndDelay,
		false);
}

void ABattleManager::PushAndStartTurn(ACombatPawn* Combatant, ETurnType Type)
{
	if (!Combatant || Combatant->GetCombatPawnState() == ECombatPawnState::Defeated) return;

	TurnStack.Emplace(Combatant, Type);
	bool bIsInterruptTurn = (Type == ETurnType::Interrupt);
	Combatant->GetBattleTurnComponent()->StartTurn(bIsInterruptTurn);

	bool bHasDoT = false;
	if (Combatant->GetStatusEffectComponent())
	{
		bHasDoT = Combatant->GetStatusEffectComponent()->HasDamageOverTimeEffect();
	}

	if (CameraComponent)
	{
		if (bHasDoT)
		{
			CameraComponent->PlayShot(StatusEffectFocusShotName, Combatant, nullptr);
		}
	}

	if (Combatant->GetStatusEffectComponent())
	{
		Combatant->GetStatusEffectComponent()->OnTurnStarted();
	}

	GetWorldTimerManager().SetTimer(
		TurnStartSequenceTimerHandle,
		this,
		&ABattleManager::OnTurnStartSequenceFinished,
		TurnStartSequenceDelay,
		false
	);

	
	OnTurnOrderChanged();

	bWaitingForTasksToStartTurn = true;
}

void ABattleManager::OnTurnStartSequenceFinished()
{
	// 1. 현재 턴인 캐릭터를 가져옵니다.
	ACombatPawn* Combatant = GetCurrentTurnCharacter();
	if (!Combatant || Combatant->GetCombatPawnState() == ECombatPawnState::Defeated)
	{
		EndCurrentTurn();
		return;
	}
	if (TurnStack.IsEmpty())
	{
		EndCurrentTurn();
		return;
	}
	const FTurnContext& CurrentTurn = TurnStack.Last();

	if (Combatant->GetFaction() == EFaction::Player && CurrentTurn.TurnType == ETurnType::Normal)
	{
		if (UTimeForceSubsystem* TimeManager = GetGameInstance()->GetSubsystem<UTimeForceSubsystem>())
		{
			if (!TimeManager->DecreaseTimeForce(1))
			{
				// 시간의 힘 소모 실패 (게임 오버됨)
				// TimeForceSubsystem이 ReturnToHub를 호출했으므로, 전투를 즉시 중단.
				return;
			}
		}
	}

	// 이 시점부터 플레이어가 입력을 할 수 있게 됩니다.
	TArray<ACombatPawn*> Targets;
	const EFaction TargetFaction = (Combatant->GetFaction() == EFaction::Player) ? EFaction::Enemy : EFaction::Player;
	for (ACombatPawn* Pawn : AllCombatants)
	{
		if (Pawn && Pawn->GetFaction() == TargetFaction && Pawn->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			Targets.Add(Pawn);
		}
	}
	UpdateInputModeForTurn(Combatant);
	Combatant->OnTurnBegin(Targets);
}

void ABattleManager::ExecuteParrySequence(ACombatPawn* Attacker, ACombatPawn* Defender)
{
	if (!Attacker || !Defender) return;

	// 1. 행동 중단
	ClearTaskQueue();

	TArray<UCombatTask*> SequenceTasks;

	// 2. 'Parry_Visuals' 액션 로드
	if (UActionComponent* ActionComp = Defender->GetActionComponent())
	{
		FActionData ParryVisualData;
		if (ActionComp->GetActionData(TEXT("Parry_Visuals"), ParryVisualData))
		{
			if (ParryVisualData.GameActionClass)
			{
				UGameAction* VisualAction = NewObject<UGameAction>(this, ParryVisualData.GameActionClass);
				VisualAction->Initialize(ActionComp, TEXT("Parry_Visuals"));

				for (UCombatTask* TaskTemplate : VisualAction->GetTasks())
				{
					if (TaskTemplate)
					{
						UCombatTask* NewTask = DuplicateObject<UCombatTask>(TaskTemplate, this);

						// [컨텍스트 설정]
						// 기본적으로 Instigator=플레이어, Target=몬스터
						// bApplyToTarget이 true면 Instigator=몬스터, Target=플레이어
						ACombatPawn* RealInstigator = TaskTemplate->bApplyToTarget ? Attacker : Defender;
						TArray<ACombatPawn*> RealTargets = TaskTemplate->bApplyToTarget ? TArray<ACombatPawn*>{ Defender } : TArray<ACombatPawn*>{ Attacker };

						NewTask->Initialize(this, RealInstigator, RealTargets);
						SequenceTasks.Add(NewTask);
					}
				}
			}
		}
	}

	// 3. 실행
	InjectCombatTasks(SequenceTasks);
}


void ABattleManager::FinalizeParryTurnSwitch(ACombatPawn* OriginalAttacker, ACombatPawn* ParryWinner)
{
	UE_LOG(LogTemp, Log, TEXT("[Parry Logic] Visuals Finished. Switching Turn & Auto-Counter."));

	// 1. 몬스터(Attacker) 턴 종료 (Pop)
	if (GetCurrentTurnCharacter() == OriginalAttacker)
	{
		TurnStack.Pop();
		OriginalAttacker->GetBattleTurnComponent()->EndTurn();
	}

	// 2. 플레이어(Winner) 추가 턴 시작 (Interrupt)
	PushAndStartTurn(ParryWinner, ETurnType::Interrupt);

	// 3. 자동 반격 스킬 실행
	if (UActionComponent* PlayerActionComp = ParryWinner->GetActionComponent())
	{
		// 실제로는 무기 데이터 등에서 가져와야 할 반격 스킬 ID
		FName CounterActionID = TEXT("Counter_Attack_Default");

		// 액션 실행 (태스크 생성 -> 큐 주입)
		bool bStarted = PlayerActionComp->StartActionByID(ParryWinner, CounterActionID, { OriginalAttacker });

		if (bStarted)
		{
			// BattleManager가 입력 대기 상태(UI 띄우기 등)로 빠지지 않도록 플래그 해제
			// (PushAndStartTurn 내부에서 bWaitingForTasksToStartTurn = true로 설정했을 것이므로)
			bWaitingForTasksToStartTurn = false;
			ParryWinner->SetCombatPawnState(ECombatPawnState::PerformingAction);
		}
		else
		{
			// 반격 액션 실행 실패 시 턴 종료 처리
			EndCurrentTurn();
		}
	}
}

void ABattleManager::EndCurrentTurn()
{
	if (TurnStack.IsEmpty()) return;

	// 1. 현재 턴 정보 가져오기 (Pop 전에!)
	ACombatPawn* EndedTurnCombatant = TurnStack.Last().Combatant;
	ETurnType EndedTurnType = TurnStack.Last().TurnType; // << 끝나는 턴의 타입을 저장
	TurnStack.Pop(); // 스택에서 제거

	// 2. 끝난 턴 처리
	if (EndedTurnCombatant)
	{
		EndedTurnCombatant->GetBattleTurnComponent()->EndTurn();
	}

	// --- [수정된 다음 행동 결정 로직] ---
	// 3. 끝난 턴이 'Interrupt'였거나 스택이 비었으면 다음 턴 결정
	if (EndedTurnType == ETurnType::Interrupt)
	{
		UE_LOG(LogTemp, Log, TEXT("Interrupt turn ended."));

		// 4. (핵심 수정)
		//    인터럽트 턴이 끝났다면, 그 밑에 깔려있던 '취소된' 일반 턴도 스택에서 제거합니다.
		if (!TurnStack.IsEmpty() && TurnStack.Last().TurnType == ETurnType::Normal)
		{
			ACombatPawn* CancelledPawn = TurnStack.Last().Combatant;
			UE_LOG(LogTemp, Warning, TEXT("Removing cancelled normal turn for %s from stack."), CancelledPawn ? *CancelledPawn->GetName() : TEXT("nullptr"));
			TurnStack.Pop(); // '좀비 턴' 제거
		}

		// 5. 스택이 완전히 정리되었으므로, 다음 일반 턴을 결정합니다.
		DecideAndStartNextTurn();
	}
	else if (TurnStack.IsEmpty())
	{
		// 6. 일반 턴이 끝났고 스택이 비었다면, 다음 일반 턴을 결정합니다.
		UE_LOG(LogTemp, Log, TEXT("Normal turn ended and stack is empty. Deciding next normal turn."));
		DecideAndStartNextTurn();
	}
	else
	{
		// 7. (예외적 상황) 스택에 남은 턴이 있고, 끝난 턴이 Interrupt가 아니었을 경우
		//    (이 분기는 이제 거의 실행되지 않아야 합니다.)
		ACombatPawn* ResumedCombatant = TurnStack.Last().Combatant;
		if (ResumedCombatant && ResumedCombatant->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			// 이전 턴 재개 (이 경우는 중첩 인터럽트 등 복잡한 상황)
			UE_LOG(LogTemp, Log, TEXT("Resuming previous turn for %s (Non-interrupt end)"), *ResumedCombatant->GetName());
			UpdateInputModeForTurn(ResumedCombatant);
		}
		else
		{
			// 재개할 턴의 캐릭터가 죽었으면 다음 턴 결정
			UE_LOG(LogTemp, Warning, TEXT("Resumed combatant %s is defeated or invalid after non-interrupt end. Deciding next turn."), ResumedCombatant ? *ResumedCombatant->GetName() : TEXT("nullptr"));
			DecideAndStartNextTurn();
		}
	}
	// --- [수정된 로직 끝] ---

	// 5. 턴 순서 UI 갱신 (항상 호출)
	OnTurnOrderChanged();
}

void ABattleManager::CheckBattleEndConditions()
{
	if (CurrentBattleState != EBattleState::InProgress) return;

	bool bAllPlayersDefeated = true;
	bool bAllEnemiesDefeated = true;

	for (const ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			if (Combatant->GetFaction() == EFaction::Player)
			{
				bAllPlayersDefeated = false;
			}
			else if (Combatant->GetFaction() == EFaction::Enemy)
			{
				bAllEnemiesDefeated = false;
			}
		}
	}

	if (bAllPlayersDefeated)
	{
		UE_LOG(LogTemp, Warning, TEXT("======= BATTLE ENDED - DEFEAT ======="));
		bPlayerWonBattle = false;
		EndBattle();
	}
	else if (bAllEnemiesDefeated)
	{
		UE_LOG(LogTemp, Warning, TEXT("======= BATTLE ENDED - VICTORY ======="));
		bPlayerWonBattle = true;
		EndBattle();
	}
}

ACombatPawn* ABattleManager::GetCurrentTurnCharacter() const
{
	return TurnStack.IsEmpty() ? nullptr : TurnStack.Last().Combatant;
}

void ABattleManager::HandleActionFinished(ACombatPawn* FinishedPawn)
{
	for (int32 i = AllCombatants.Num() - 1; i >= 0; --i)
	{
		ACombatPawn* Pawn = AllCombatants[i];
		if (IsValid(Pawn))
		{
			Pawn->ExecuteDelayedDeath();
		}
	}

	// 기존 로직 (턴 종료 처리 등)
	if (GetCurrentTurnCharacter() == FinishedPawn)
	{
		EndCurrentTurn();
	}
}

void ABattleManager::HandleInterruptRequest(ACombatPawn* InInstigator)
{
	UE_LOG(LogTemp, Warning, TEXT("Interrupt Turn Requested by %s!"), *InInstigator->GetName());
	PushAndStartTurn(InInstigator, ETurnType::Interrupt);
}

void ABattleManager::HandleCombatantDied(AActor* Victim, AActor* InInstigator)
{
	ACombatPawn* DeadPawn = Cast<ACombatPawn>(Victim);
	if (!DeadPawn) return;

	if (DeadPawn->GetFaction() == EFaction::Player)
	{
		if (UTimeForceSubsystem* TimeManager = GetGameInstance()->GetSubsystem<UTimeForceSubsystem>())
		{
			// 시간의 힘 20 소모 시도
			if (TimeManager->DecreaseTimeForce(20))
			{
				DeadPawn->ReviveFromDefeat(0.5f);
				UE_LOG(LogTemp, Warning, TEXT("플레이어 부활 성공! (시간의 힘 소모)"));

				// 부활했으므로 패배 조건 체크 없이 리턴
				return;
			}
			else
			{
				// [실패] 시간의 힘 부족 -> 게임 오버
				UE_LOG(LogTemp, Error, TEXT("부활 실패: 시간의 힘 부족."));
			}
		}
	}
	// ---

	CheckBattleEndConditions();
}

void ABattleManager::HandleParryAttempted(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult)
{
	if (ParryResult == EParryResult::Success)
	{
		ExecuteParrySequence(ParriedAttacker, ParryingPlayer);
	}
}

void ABattleManager::HandleDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, EDamageFloaterType DamageType, ACombatPawn* InstigatorPawn)
{
	if (!DamagedPawn || DamageAmount <= 0.f)
	{
		return;
	}
	
	DamagedPawn->K2_ShowDamageFloater(DamageAmount, DamageType);
}

void ABattleManager::HandleEnemyParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration)
{
	for (UPlayerCombatControlComponent* ControlComp : PlayerControlComponents)
	{
		if (ControlComp) ControlComp->OnReceiveParryWindowOpened(Attacker, AttackType, Duration);
	}
}

void ABattleManager::HandleEnemyParryWindowClosed(ACombatPawn* Attacker)
{
	for (UPlayerCombatControlComponent* ControlComp : PlayerControlComponents)
	{
		if (ControlComp) ControlComp->OnReceiveParryWindowClosed(Attacker);
	}
}

void ABattleManager::DecideAndStartNextTurn()
{
	if (CurrentBattleState != EBattleState::InProgress) return;

	ACombatPawn* NextTurnPawn = TurnScheduler->DetermineNextTurnCombatant(AllCombatants);

	if (NextTurnPawn)
	{
		PushAndStartTurn(NextTurnPawn, ETurnType::Normal);
	}
	else
	{
		CheckBattleEndConditions();
	}
}

void ABattleManager::UpdateInputModeForTurn(ACombatPawn* TurnCombatant)
{
	if (CachedPlayerController && TurnCombatant)
	{
		if (TurnCombatant->GetFaction() == EFaction::Player)
		{
			CachedPlayerController->SetPlayerTurnInputMode(); 
		}
		else if (TurnCombatant->GetFaction() == EFaction::Enemy)
		{
			CachedPlayerController->SetEnemyTurnInputMode(); 
		}
	}
}

void ABattleManager::QueueUpCombatTasks(const TArray<UCombatTask*>& Tasks)
{
	TaskQueue.Append(Tasks);
}

void ABattleManager::InjectCombatTasks(const TArray<UCombatTask*>& Tasks)
{
	if (Tasks.Num() > 0)
	{
		TaskQueue.Insert(Tasks, 0);
	}
}

void ABattleManager::ClearTaskQueue()
{
	if (CurrentTask)
	{
		ACombatPawn* InInstigator = CurrentTask->GetInstigator();
		if (InInstigator)
		{
			InInstigator->StopAnimMontage();
		}
		if (CurrentTask->OnTaskFinished.IsBound())
		{
			CurrentTask->OnTaskFinished.RemoveDynamic(this, &ABattleManager::OnCurrentTaskFinished);
		}
	}
	CurrentTask = nullptr;
	bIsProcessingTask = false;
	TaskQueue.Empty();
}

void ABattleManager::SignalTaskByNotifyName(FName NotifyName)
{
	if (CurrentTask)
	{
		CurrentTask->OnNotifyReceived(NotifyName);
	}
}

void ABattleManager::RequestPlayerInterruptTurn(ACombatPawn* PlayerPawn)
{
	if (PlayerPawn && PlayerPawn->GetFaction() == EFaction::Player)
	{
		UE_LOG(LogTemp, Log, TEXT("BattleManager: Received Interrupt Turn Request from %s."), *PlayerPawn->GetName());
		// 인터럽트 타입으로 턴 스택에 추가하고 즉시 시작
		PushAndStartTurn(PlayerPawn, ETurnType::Interrupt);
	}
}

void ABattleManager::ProcessTaskQueue()
{
	if (bIsProcessingTask || TaskQueue.Num() == 0) return;

	CurrentTask = TaskQueue[0];
	TaskQueue.RemoveAt(0);

	if (CurrentTask)
	{
		bIsProcessingTask = true;
		CurrentTask->OnTaskFinished.AddDynamic(this, &ABattleManager::OnCurrentTaskFinished);
		CurrentTask->ExecuteTask();
	}
}

void ABattleManager::OnCurrentTaskFinished()
{
	bIsProcessingTask = false;
	CurrentTask = nullptr;
}

void ABattleManager::TriggerFieldTransition()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBattleTransitionManagerSubsystem* TransitionManager = GameInstance->GetSubsystem<UBattleTransitionManagerSubsystem>())
		{
			TransitionManager->RequestExitBattle(bPlayerWonBattle);
		}
	}
}

