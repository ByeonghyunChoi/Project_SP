#include "BattleManager.h"
#include "PlayerCharacter.h"
#include "MonsterCharacter.h"

ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	GlobalTime = 0.f;
	CurrentBattleState = EBattleState::Setup;
	CurrentTurnCharacter = nullptr;
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentBattleState == EBattleState::InProgress)
	{
		ProcessTurn();
	}
}

void ABattleManager::StartBattle(const TArray<ACombatPawn*>& InitialCombatants)
{
	AllCombatants = InitialCombatants;
	CurrentBattleState = EBattleState::InProgress;
	GlobalTime = 0.0f;

	if (OnTurnOrderChanged.IsBound())
	{
		OnTurnOrderChanged.Broadcast();
	}
		
}

void ABattleManager::AddCombatant(ACombatPawn* NewCombatant)
{
	if (NewCombatant)
	{
		AllCombatants.AddUnique(NewCombatant);
	}
}

TArray<ACombatPawn*> ABattleManager::GetReadyCombatants() const
{
	TArray<ACombatPawn*> Ready;
	for (ACombatPawn* Pawn : AllCombatants)
	{
		auto Stats = Pawn->GetStatsComponent();
		auto Turn = Pawn->GetBattleTurnComponent();
		if (Stats && Turn && Stats->GetCurrentHealth() > 0 && Turn->IsReadyForTurn())
		{
			Ready.Add(Pawn);
		}
	}
	return Ready;
}

float ABattleManager::GetMinTimeToNextTurn() const
{
	float MinTime = TNumericLimits<float>::Max();
	for (ACombatPawn* Pawn : AllCombatants)
	{
		auto Stats = Pawn->GetStatsComponent();
		auto Turn = Pawn->GetBattleTurnComponent();
		if (Stats && Turn && Stats->GetCurrentHealth() > 0 && !Turn->GetIsMyTurn())
		{
			float TimeLeft = Turn->GetTimeLeftToAct();
			if (TimeLeft < MinTime)
				MinTime = TimeLeft;
		}
	}
	return MinTime;
}

void ABattleManager::AdvanceAllActionValues(float DeltaTime)
{
	for (ACombatPawn* Pawn : AllCombatants)
	{
		if (auto Turn = Pawn->GetBattleTurnComponent())
		{
			Turn->AdvanceActionValue(DeltaTime);
		}
	}
}

void ABattleManager::ProcessTurn()
{
	if (CheckBattleEndConditions())
	{
		EndBattle();
		return;
	}

	TArray<ACombatPawn*> Ready = GetReadyCombatants();
	if (Ready.Num() > 0)
	{
		Ready.Sort([](const ACombatPawn& A, const ACombatPawn& B) {
			return ABattleManager::CombatantSortPredicate(A, B);
			});
		InitiateTurnFor(Ready[0]);
	}
	else
	{
		float WaitTime = GetMinTimeToNextTurn();
		GlobalTime += WaitTime;
		AdvanceAllActionValues(WaitTime);
	}
}

void ABattleManager::InitiateTurnFor(ACombatPawn* Target)
{
	if (!Target)
		return;

	CurrentTurnCharacter = Target;

	if (auto Turn = Target->GetBattleTurnComponent())
		Turn->StartTurn();

	EFaction Faction = Target->GetFaction();
	if (Faction == EFaction::Player)
	{
		CurrentBattleState = EBattleState::PlayerTurn;
	}
	else
	{
		CurrentBattleState = EBattleState::EnemyTurn;
		if (auto Monster = Cast<AMonsterCharacter>(Target))
		{
			//몬스터 행동 처리 AI 로직
		}
	}
}

void ABattleManager::EndTurn()
{
	if (CurrentTurnCharacter)
	{
		if (auto Turn = CurrentTurnCharacter->GetBattleTurnComponent())
			Turn->EndTurn();
	}

	CurrentTurnCharacter = nullptr;
	CurrentBattleState = EBattleState::InProgress;

	if (OnTurnOrderChanged.IsBound())
		OnTurnOrderChanged.Broadcast();
}

bool ABattleManager::CheckBattleEndConditions() const
{
	int32 PlayerCount = 0;
	int32 EnemyCount = 0;

	for (ACombatPawn* Pawn : AllCombatants)
	{
		if (auto Stats = Pawn->GetStatsComponent())
		{
			if (Stats->GetCurrentHealth() > 0)
			{
				if (Pawn->GetFaction() == EFaction::Player)
					PlayerCount++;
				else if (Pawn->GetFaction() == EFaction::Enemy)
					EnemyCount++;
			}
		}
	}

	if (PlayerCount == 0 || EnemyCount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("전투 종료: 플레이어 %d명, 적 %d명"), PlayerCount, EnemyCount);
		return true;
	}
	return false;
}

void ABattleManager::EndBattle()
{
	CurrentBattleState = EBattleState::Ended;
	UE_LOG(LogTemp, Log, TEXT("전투 종료"));
}

bool ABattleManager::CombatantSortPredicate(const ACombatPawn& A, const ACombatPawn& B)
{
	float TimeA = A.GetBattleTurnComponent()->GetTimeLeftToAct();
	float TimeB = B.GetBattleTurnComponent()->GetTimeLeftToAct();

	if (TimeA != TimeB)
		return TimeA < TimeB;

	float SpeedA = A.GetStatsComponent()->GetMovementSpeed();
	float SpeedB = B.GetStatsComponent()->GetMovementSpeed();

	if (SpeedA != SpeedB)
		return SpeedA > SpeedB;

	// 플레이어 우선
	if (A.GetFaction() == EFaction::Player && B.GetFaction() != EFaction::Player) return true;
	if (B.GetFaction() == EFaction::Player && A.GetFaction() != EFaction::Player) return false;

	// UID로 최종 비교
	return A.GetUniqueID() < B.GetUniqueID();
}

const TArray<ACombatPawn*>& ABattleManager::GetAllCombatants() const
{
	return AllCombatants;
}

ACombatPawn* ABattleManager::GetCurrentTurnCharacter() const
{
	return CurrentTurnCharacter;
}

