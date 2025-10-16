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

	if (TurnStack.IsEmpty())
	{
		DecideAndStartNextTurn();
	}

	ProcessTaskQueue();
}

void ABattleManager::StartBattle(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
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
}

void ABattleManager::PushAndStartTurn(ACombatPawn* Combatant, ETurnType Type)
{
	if (!Combatant || Combatant->GetCombatPawnState() == ECombatPawnState::Defeated) return;

	TurnStack.Emplace(Combatant, Type);
	Combatant->GetBattleTurnComponent()->StartTurn();

	TArray<ACombatPawn*> Targets;
	const EFaction TargetFaction = (Combatant->GetFaction() == EFaction::Player) ? EFaction::Enemy : EFaction::Player;
	for (ACombatPawn* Pawn : AllCombatants)
	{
		if (Pawn && Pawn->GetFaction() == TargetFaction && Pawn->GetCombatPawnState() != ECombatPawnState::Defeated)
		{
			Targets.Add(Pawn);
		}
	}
	Combatant->OnTurnBegin(Targets);
	OnTurnOrderChanged();
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

	OnTurnOrderChanged();

	DecideAndStartNextTurn();
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
		EndBattle();
	}
	else if (bAllEnemiesDefeated)
	{
		UE_LOG(LogTemp, Warning, TEXT("======= BATTLE ENDED - VICTORY ======="));
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

