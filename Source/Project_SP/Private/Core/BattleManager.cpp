// Core/BattleManager.cpp

#include "Core/BattleManager.h"
#include "Character/CombatPawn.h"
#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"
#include "Component/GameEventComponent.h"
#include "Component/CombatCameraComponent.h"
#include "Component/TurnSchedulerComponent.h"
#include "Combat/CombatTask.h"
#include "Combat/Tasks/Task_WaitForAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Character/MyPlayerController.h"
#include "TimerManager.h"
#include "Combat/BattleTransitionManager.h"
#include "Component/StatusEffectComponent.h"

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

	if (bIsProcessingTask && CurrentTask && CurrentTask->IsLatent())
	{
		CurrentTask->TickTask(DeltaTime);
	}

	if (TurnStack.IsEmpty())
	{
		DecideAndStartNextTurn();
	}

	ProcessTaskQueue();
}

void ABattleManager::StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
	CachedPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	AllCombatants.Empty();
	AllCombatants.Append(PlayerParty);
	AllCombatants.Append(EnemyParty);

	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant)
		{
			if (Combatant->GetGameEventComponent())
			{
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
	Combatant->GetBattleTurnComponent()->StartTurn();

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

void ABattleManager::EndCurrentTurn()
{
	if (TurnStack.IsEmpty()) return;

	ACombatPawn* EndedTurnCombatant = TurnStack.Last().Combatant;
	TurnStack.Pop();

	if (EndedTurnCombatant)
	{
		EndedTurnCombatant->GetBattleTurnComponent()->EndTurn();
	}

	if (TurnStack.Num() > 0)
	{
		ACombatPawn* ResumedCombatant = TurnStack.Last().Combatant;
		if (ResumedCombatant)
		{
			// 중단되었던 턴의 입력 모드(IMC)를 다시 활성화합니다.
			UpdateInputModeForTurn(ResumedCombatant);
		}
	}
	else
	{
		DecideAndStartNextTurn();
	}

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

void ABattleManager::HandleCombatantDied(AActor* InInstigator)
{
	CheckBattleEndConditions();
}

void ABattleManager::HandleParryAttempted(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult)
{
}

void ABattleManager::HandleDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, EDamageFloaterType DamageType, ACombatPawn* InstigatorPawn)
{
	if (!DamagedPawn || DamageAmount <= 0.f)
	{
		return;
	}

	DamagedPawn->K2_ShowDamageFloater(DamageAmount, DamageType);
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
	TaskQueue.Insert(Tasks, 0);
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
	if (UTask_WaitForAnimNotify* WaitTask = Cast<UTask_WaitForAnimNotify>(CurrentTask))
	{
		// 대기 중인 작업에게 신호를 전달합니다.
		WaitTask->OnNotifyReceived(NotifyName);
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
		if (UBattleTransitionManager* TransitionManager = GameInstance->GetSubsystem<UBattleTransitionManager>())
		{
			TransitionManager->RequestExitBattle(bPlayerWonBattle);
		}
	}
}

