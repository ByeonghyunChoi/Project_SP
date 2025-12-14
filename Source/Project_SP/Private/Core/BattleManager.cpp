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
#include "Data/WeaponData.h"
#include "Component/WeaponSystemComponent.h"
#include "SubSystem/SoundManagerSubsystem.h"

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

	if (TaskQueue.Num() > 0 && !bIsProcessingTask)
	{
		ProcessTaskQueue();
	}

	// 1. 현재 실행 중인 태스크가 있다면 틱 공급
	if (bIsProcessingTask && CurrentTask)
	{
		if (CurrentTask->IsLatent())
		{
			CurrentTask->TickTask(DeltaTime);
		}
		return; // 태스크 실행 중에는 아래 로직 수행 안 함
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

	if (USoundManagerSubsystem* SoundMgr = GetGameInstance()->GetSubsystem<USoundManagerSubsystem>())
	{
		SoundMgr->SwitchToBattleBGM(DefaultBattleBGM);
	}
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

	if (USoundManagerSubsystem* SoundMgr = GetGameInstance()->GetSubsystem<USoundManagerSubsystem>())
	{
		SoundMgr->ReturnToFieldBGM();
	}
	
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
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TurnStartSequenceTimerHandle);
	}

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

	// 1. 데이터 저장 (나중에 써야 함)
	PendingParryAttacker = Attacker;
	PendingParryDefender = Defender;

	// 2. 적 행동 중단
	ClearTaskQueue();
	Attacker->StopAnimMontage();

	// 3. [시각적 연출 시작] Parry_Visual
	if (UActionComponent* PlayerActionComp = Defender->GetActionComponent())
	{
		PlayerActionComp->StartActionByName(Defender, TEXT("Parry_Visual"));
	}
}

void ABattleManager::ActivateHitStop(float Duration, float Dilation)
{
	UE_LOG(LogTemp, Warning, TEXT(">>> ActivateHitStop Called. Duration: %f, Dilation: %f"), Duration, Dilation);

	// 1. 시간 정지 적용
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), Dilation);

	// 2. 타이머 지연 시간 계산
	// 목표: 현실 시간(RealTime)으로 Duration만큼 멈추고 싶다.
	// 공식: GameTime = RealTime * Dilation
	float GameTimeDelay = Duration * Dilation;

	// [안전장치] 지연 시간이 너무 작으면(0에 수렴하면) 타이머가 안 돌 수도 있으므로 최소값 보장
	// 0.001초(GameTime)보다 작으면 0.001초로 설정 (프레임 틱 보장)
	if (GameTimeDelay < 0.001f)
	{
		GameTimeDelay = 0.001f;
	}

	UE_LOG(LogTemp, Warning, TEXT(">>> Timer Set for: %f seconds (GameTime)"), GameTimeDelay);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &ABattleManager::OnHitStopFinished, GameTimeDelay, false);
}

void ABattleManager::OnHitStopFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("<<< OnHitStopFinished Called. Restoring Time..."));

	// 1. 시간 복구
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 2. 후속 조치 (태스크 병합 및 주입)
	if (PendingParryAttacker && PendingParryDefender)
	{
		TArray<UCombatTask*> CombinedTasks;

		// -------------------------------------------------------
		// [Step 1] 몬스터 리액션 태스크 추출 (Parry_Reaction)
		// -------------------------------------------------------
		// 몬스터가 "으악!" 하고 뒤로 물러나는 동작
		if (UActionComponent* MonsterActionComp = PendingParryAttacker->GetActionComponent())
		{
			FActionData ReactData;
			// 데이터 테이블에서 'Parry_Reaction'을 찾음
			if (MonsterActionComp->GetActionData(TEXT("Parry_Reaction"), ReactData))
			{
				UGameAction* TempAction = NewObject<UGameAction>(this, ReactData.GameActionClass);
				if (TempAction)
				{
					for (UCombatTask* TaskTemplate : TempAction->GetTasks())
					{
						UCombatTask* NewTask = DuplicateObject<UCombatTask>(TaskTemplate, this);

						// Instigator: 몬스터 (자기가 움직여야 하니까)
						NewTask->Initialize(this, PendingParryAttacker, {});

						CombinedTasks.Add(NewTask);
					}
				}
			}
		}

		// -------------------------------------------------------
		// [Step 2] 턴 교체 태스크 추출 (Turn_Switch)
		// -------------------------------------------------------
		// 몬스터가 다 물러난 뒤에(위 태스크가 끝나면) 실행됨
		if (UActionComponent* PlayerActionComp = PendingParryDefender->GetActionComponent())
		{
			FActionData SwitchData;
			// 데이터 테이블에서 'Turn_Switch'를 찾음
			if (PlayerActionComp->GetActionData(TEXT("Turn_Switch"), SwitchData))
			{
				UGameAction* TempAction = NewObject<UGameAction>(this, SwitchData.GameActionClass);
				if (TempAction)
				{
					for (UCombatTask* TaskTemplate : TempAction->GetTasks())
					{
						UCombatTask* NewTask = DuplicateObject<UCombatTask>(TaskTemplate, this);

						// Instigator: 몬스터 (몬스터 턴을 끝내야 하므로)
						// Target: 플레이어 (플레이어에게 턴을 줘야 하므로)
						NewTask->Initialize(this, PendingParryAttacker, { PendingParryDefender });

						CombinedTasks.Add(NewTask);
					}
				}
			}
		}

		// -------------------------------------------------------
		// [Step 3] 통합 주입 (순차 실행 보장)
		// -------------------------------------------------------
		if (CombinedTasks.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> Injecting %d Combined Tasks"), CombinedTasks.Num());

			// 큐의 맨 앞에 넣습니다. (새치기)
			// TaskQueue는 FIFO(선입선출)이므로, 넣은 순서대로 [리액션 -> 턴스위치]가 실행됩니다.
			// 리액션(Move)이 Latent라면, 그게 끝날 때까지 턴스위치는 대기합니다.
			InjectCombatTasks(CombinedTasks);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("!!! No Tasks Found! Check DataTable Row Names (Parry_Reaction, Turn_Switch) !!!"));
		}
	}

	// 데이터 초기화
	PendingParryAttacker = nullptr;
	PendingParryDefender = nullptr;
}


void ABattleManager::FinalizeParryTurnSwitch(ACombatPawn* OriginalAttacker, ACombatPawn* ParryWinner)
{
	UE_LOG(LogTemp, Warning, TEXT(">>> FinalizeParryTurnSwitch Called."));

	// 1. 적 턴 종료
	if (GetCurrentTurnCharacter() == OriginalAttacker)
	{
		TurnStack.Pop();
		OriginalAttacker->GetBattleTurnComponent()->EndTurn();
	}

	// 2. 플레이어 추가 턴 시작
	PushAndStartTurn(ParryWinner, ETurnType::Interrupt);

	// 턴 시작 대기 취소
	bWaitingForTasksToStartTurn = false;
	GetWorld()->GetTimerManager().ClearTimer(TurnStartSequenceTimerHandle);

	// 3. 자동 반격 스킬 실행
	bool bActionStarted = false;

	if (UWeaponSystemComponent* WeaponComp = ParryWinner->FindComponentByClass<UWeaponSystemComponent>())
	{
		if (UWeaponData* Weapon = WeaponComp->GetCurrentWeapon())
		{
			FName CounterActionID = Weapon->ParrySkillActionID;

			UE_LOG(LogTemp, Warning, TEXT("   > Attempting to start Counter Action: %s"), *CounterActionID.ToString());

			if (!CounterActionID.IsNone())
			{
				// [핵심 수정] ★★★
				// 현재 플레이어는 'Turn_Switch' 액션을 수행 중인 상태(Active)일 수 있습니다.
				// 새 스킬을 쓰려면 기존 상태를 강제로 초기화해야 합니다.
				if (UActionComponent* ActionComp = ParryWinner->GetActionComponent())
				{
					ActionComp->ResetActiveAction(); // "야, 하던 거(Turn_Switch) 잊어버려!"

					// 이제 깨끗한 상태에서 반격기 실행
					bActionStarted = ActionComp->StartActionByID(ParryWinner, CounterActionID, { OriginalAttacker });
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("   > ERROR: ParrySkillActionID is NONE!"));
			}
		}
	}

	// 4. 실패 시 안전장치
	if (!bActionStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("!!! Counter Action Failed to Start! Ending Turn Forcefully. !!!"));
		// 여기서 EndCurrentTurn을 하면 턴이 꼬일 수 있으니, 
		// 차라리 상태를 AwaitingInput으로 바꿔서 플레이어가 수동으로라도 때리게 해줍니다.
		ParryWinner->SetCombatPawnState(ECombatPawnState::AwaitingInput);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(">>> Counter Action Started Successfully!"));
		ParryWinner->SetCombatPawnState(ECombatPawnState::PerformingAction);
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
	UE_LOG(LogTemp, Warning, TEXT("BattleManager received Signal: %s"), *NotifyName.ToString());

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
		UE_LOG(LogTemp, Warning, TEXT(">>> ProcessTask: Executing [%s]"), *CurrentTask->GetName());

		bIsProcessingTask = true;
		CurrentTask->OnTaskFinished.AddDynamic(this, &ABattleManager::OnCurrentTaskFinished);
		CurrentTask->ExecuteTask();
	}
}

void ABattleManager::OnCurrentTaskFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("<<< Task Finished."));

	if (CurrentTask)
	{
		CurrentTask->OnTaskFinished.RemoveDynamic(this, &ABattleManager::OnCurrentTaskFinished);
	}

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

