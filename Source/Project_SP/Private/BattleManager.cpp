#include "BattleManager.h"
#include "PlayerCharacter.h"
#include "MonsterCharacter.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	GlobalTime = 0.f;
	CurrentBattleState = EBattleState::Setup;
	CurrentTurnCharacter = nullptr;
	PlayerControllerRef = nullptr;
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();

	// BeginPlay에서 PlayerControllerRef 초기화
	// GetWorld()->GetFirstPlayerController()를 통해 현재 레벨의 첫 번째 플레이어 컨트롤러를 가져옴
	if (GetWorld())
	{
		PlayerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	}

	// 플레이어 컨트롤러를 찾았으면 초기 입력 설정 함수 호출 (블루프린트에서 구현)
	if (PlayerControllerRef)
	{
		InitializeCombatInput(PlayerControllerRef);
	}
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

	if (CurrentBattleState == EBattleState::InProgress)
	{
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
			if (CurrentBattleState == EBattleState::InProgress)
			{
				float WaitTime = GetMinTimeToNextTurn();
				GlobalTime += WaitTime;
				AdvanceAllActionValues(WaitTime);
			}
			
		}
	}
}

void ABattleManager::InitiateTurnFor(ACombatPawn* Target)
{
	if (!Target)
		return;

	CurrentTurnCharacter = Target;

	if (auto Turn = Target->GetBattleTurnComponent())
	{
		Turn->StartTurn();
	}
		
	EFaction Faction = Target->GetFaction();
	if (Faction == EFaction::Player)
	{
		CurrentBattleState = EBattleState::PlayerTurn;
		if (PlayerControllerRef)
		{
			ActivatePlayerInput(PlayerControllerRef); // 블루프린트에서 구현될 플레이어 입력 활성화 함수 호출
		}
	}
	else
	{
		CurrentBattleState = EBattleState::EnemyTurn;
		if (PlayerControllerRef)
		{
			DeactivatePlayerInput(PlayerControllerRef); // 블루프린트에서 구현될 플레이어 입력 비활성화 함수 호출
		}

		if (auto Monster = Cast<AMonsterCharacter>(Target))
		{
			ACombatPawn* PlayerPawn = nullptr;
			for (ACombatPawn* Combatant : AllCombatants)
			{
				if (Combatant->GetFaction() == EFaction::Player &&
					Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
				{
					PlayerPawn = Combatant;
					break;
				}
			}
			if (PlayerPawn)
			{
				CurrentBattleState = EBattleState::ExecutingAction;
				Monster->TriggerAttackBlueprintEvent();
			}
			else
			{
				EndTurn();
			}
		}
	}

	if (OnTurnOrderChanged.IsBound())
		OnTurnOrderChanged.Broadcast();
}

void ABattleManager::EndTurn()
{
	if (PlayerControllerRef)
	{
		DeactivatePlayerInput(PlayerControllerRef); // 블루프린트에서 구현될 플레이어 입력 비활성화 함수 호출
	}

	if (CurrentTurnCharacter)
	{
		if (auto Turn = CurrentTurnCharacter->GetBattleTurnComponent())
			Turn->EndTurn();
	}

	CurrentTurnCharacter = nullptr;
	CurrentBattleState = EBattleState::InProgress;
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

	if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
	{
		GI->ReturnToFieldTransition(true); 
	}
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



