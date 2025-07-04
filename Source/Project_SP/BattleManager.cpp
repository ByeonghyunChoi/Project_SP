// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleManager.h"
#include "MonsterCharacter.h" 
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABattleManager::ABattleManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CurrentBattleState = EBattleState::None;
	GlobalTime = 0.0f;
	CurrentTurnCharacter = nullptr;
}

// Called when the game starts or when spawned
void ABattleManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABattleManager::SortReadyCombatants(TArray<ACombatPawn*>& ReadyCombatants)
{
	// 이전에 UCharacterBase에서 논의했던 턴 우선순위 정렬 로직을 여기에 구현합니다.
	// 1. 속도 (fSpeed) 내림차순
	// 2. 속도가 같을 경우, 아군(Player) 진영 우선
	// 3. 속도와 진영이 같을 경우, 턴 순서 결정 인덱스 (iTurnOrderIndex) 오름차순
	ReadyCombatants.Sort([](const ACombatPawn& A, const ACombatPawn& B) -> bool
	{
			//캐릭터의 데이터에 접근하기 위한 변수
			UCharacterBase* CombatantAData = nullptr;
			UCharacterBase* CombatantBData = nullptr;

			//캐릭터가 몬스터인지 플레이어인지 구분
			if (const APlayerCharacter* PlayerA = Cast<const APlayerCharacter>(&A))
			{
				CombatantAData = PlayerA->GetCombatData();
			}
			else if (const AMonsterCharacter* MonsterA = Cast<const AMonsterCharacter>(&A))
			{
				CombatantAData = MonsterA->GetCombatData();
			}
			if (const APlayerCharacter* PlayerB = Cast<const APlayerCharacter>(&B))
			{
				CombatantAData = PlayerB->GetCombatData();
			}
			else if (const AMonsterCharacter* MonsterB = Cast<const AMonsterCharacter>(&B))
			{
				CombatantAData = MonsterB->GetCombatData();
			}

			//예외처리 나중에 추가할 예정
			if (!CombatantAData || !CombatantBData)
			{
				return false;
			}

			// 1. 속도 (fSpeed) 내림차순 정렬
			if (CombatantAData->GetStats().fSpeed != CombatantBData->GetStats().fSpeed)
			{
				return CombatantAData->GetStats().fSpeed > CombatantBData->GetStats().fSpeed;
			}

			// 2. 속도가 같을 경우, 아군(Player) 진영 우선
			if (CombatantAData->GetFaction() != CombatantBData->GetFaction())
			{
				return CombatantAData->GetFaction() == EFaction::Player;
			}

			// 3. 속도와 진영이 같을 경우, 턴 순서 결정 인덱스 (iTurnOrderIndex) 오름차순
			return (CombatantAData->GetTurnOrderIndex() < CombatantBData->GetTurnOrderIndex());

	});
}

bool ABattleManager::CheckBattleEndConditions()
{
	int32 PlayerSideCount = 0;
	int32 EnemySideCount = 0;

	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		if (CombatantActor && CombatantActor->IsValidLowLevel())
		{
			//Geter함수로 데이터를 가져옴
			UCharacterBase* CombatantData = nullptr;
			if (const APlayerCharacter* Player = Cast<const APlayerCharacter>(CombatantActor))
			{
				CombatantData = Player->GetCombatData();
			}
			else if (const AMonsterCharacter* Monster = Cast<const AMonsterCharacter>(CombatantActor))
			{
				CombatantData = Monster->GetCombatData();
			}

			// 유효성 및 생존 확인
			if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0) 
			{
				//Combatant의 진영을 확인하여 PlayerSideCount 또는 EnemySideCount 증가
				if (CombatantData->GetFaction() == EFaction::Player)
				{
					PlayerSideCount++;
				}
				else if (CombatantData->GetFaction() == EFaction::Enemy)
				{
					EnemySideCount++;
				}
			}
		}
	}
	// 모든 적이 쓰러지면 승리
	if (EnemySideCount == 0 && PlayerSideCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("전투 승리!"));
		return true;
	}
	// 모든 아군이 쓰러지면 패배
	else if (PlayerSideCount == 0 && EnemySideCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("전투 패배!"));
		return true;
	}

	return false; // 전투가 아직 끝나지 않음
}

//Called every frame
void ABattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentBattleState == EBattleState::InProgress ||
		CurrentBattleState == EBattleState::PlayerTurn ||
		CurrentBattleState == EBattleState::EnemyTurn)
	{
		GlobalTime += DeltaTime;

		// 모든 전투 참여자의 행동 게이지 업데이트
		for (ACombatPawn* CombatantActor : AllCombatants)
		{
			// ACombatPawn*에서 GetCombatData()를 통해 UCharacterBase*를 가져옴
			UCharacterBase* CombatantData = nullptr;
			if (const APlayerCharacter* Player = Cast<const APlayerCharacter>(CombatantActor))
			{
				CombatantData = Player->GetCombatData();
			}
			else if (const AMonsterCharacter* Monster = Cast<const AMonsterCharacter>(CombatantActor))
			{
				CombatantData = Monster->GetCombatData();
			}

			// 유효성 및 생존 확인
			if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0) 
			{
				// CombatantData를 통해 UpdateActionGauge 호출
				CombatantData->UpdateActionGauge(DeltaTime); 
			}
		}
	

		// 현재 턴 캐릭터가 없거나, 현재 턴 캐릭터가 행동을 완료했다면 다음 턴을 처리합니다.
		// (CurrentTurnCharacter가 nullptr이거나, GetIsMyTurn()이 false라면)
		if (!CurrentTurnCharacter || (CurrentTurnCharacter->GetCombatData() && !CurrentTurnCharacter->GetCombatData()->GetIsMyTurn()))
		{
			ProcessTurn();
		}
	}

}

void ABattleManager::StartBattle(TArray<ACombatPawn*> InitialCombatants)
{
	UE_LOG(LogTemp, Log, TEXT("StartBattle: 함수 진입."));
	CurrentBattleState = EBattleState::Setup;
	GlobalTime = 0.0f;
	CurrentTurnCharacter = nullptr;
	AllCombatants.Empty();
	UE_LOG(LogTemp, Log, TEXT("StartBattle: 상태를 Setup으로 변경."))

	for (ACombatPawn* CombatantActor : InitialCombatants)
	{
		AddCombatant(CombatantActor);
	}
	UE_LOG(LogTemp, Log, TEXT("StartBattle: 상태를 InProgress로 변경 직전."));
	CurrentBattleState = EBattleState::InProgress;
	UE_LOG(LogTemp, Log, TEXT("StartBattle: 상태를 InProgress로 변경 완료. 현재 상태: %s"), *UEnum::GetValueAsString(CurrentBattleState));
}

void ABattleManager::EndBattle()
{
	CurrentBattleState = EBattleState::Ended;

	//전투 결과 처리
}

void ABattleManager::ProcessTurn()
{
	if (CheckBattleEndConditions())
	{
		EndBattle();
		return;
	}

	// 턴을 획득할 준비가 된 캐릭터 목록 생성
	TArray<ACombatPawn*> ReadyCombatants;
	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();

		if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0 && CombatantData->IsReadyForTurn())
		{
			ReadyCombatants.Add(CombatantActor);
		}
	}

	if (ReadyCombatants.Num() == 0)
	{
		return;
	}

	SortReadyCombatants(ReadyCombatants);

	CurrentTurnCharacter = ReadyCombatants[0];
	if (CurrentTurnCharacter->GetCombatData())
	{
		CurrentTurnCharacter->GetCombatData()->StartTurn();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ABattleManager: CurrentTurnCharacter의 CombatData가 유효하지 않습니다."));
		return;
	}

	if (CurrentTurnCharacter->GetCombatData()->GetFaction() == EFaction::Player)
	{
		CurrentBattleState = EBattleState::PlayerTurn;
		UE_LOG(LogTemp, Log, TEXT("플레이어 턴 시작! 입력 대기 중..."));

		// APlayerCharacter로 캐스팅하여 BattleModeComp에 접근
		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CurrentTurnCharacter);
		if (PlayerChar && PlayerChar->BattleModeComp)
		{
			PlayerChar->BattleModeComp->ReceiveTurn();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 플레이어 턴이나, APlayerCharacter 또는 BattleModeComp가 유효하지 않습니다."));
		}
	}
	else // 몬스터나 NPC
	{
		CurrentBattleState = EBattleState::EnemyTurn;
		UE_LOG(LogTemp, Log, TEXT("몬스터 턴 시작! 행동 결정 중..."));

		// AMonsterCharacter로 캐스팅하여 PerformMonsterTurnAction() 호출
		AMonsterCharacter* MonsterChar = Cast<AMonsterCharacter>(CurrentTurnCharacter);
		if (MonsterChar)
		{
			MonsterChar->PerformMonsterTurnAction(); // AMonsterCharacter의 턴 행동 함수 호출
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 몬스터 턴이나, AMonsterCharacter가 유효하지 않습니다."));
		}
	}
}

void ABattleManager::AddCombatant(ACombatPawn* NewCombatant)
{
	// ACombatPawn*가 유효하고, GetCombatData()를 통해 UCharacterBase*를 얻을 수 있을 때만 추가
	if (NewCombatant && NewCombatant->IsValidLowLevel())
	{
		UCharacterBase* CombatantData = nullptr;
		if (const APlayerCharacter* Player = Cast<const APlayerCharacter>(NewCombatant))
		{
			CombatantData = Player->GetCombatData();
		}
		else if (const AMonsterCharacter* Monster = Cast<const AMonsterCharacter>(NewCombatant))
		{
			CombatantData = Monster->GetCombatData();
		}

		if (CombatantData)
		{
			AllCombatants.AddUnique(NewCombatant); // 중복 추가 방지
			UE_LOG(LogTemp, Log, TEXT("전투 참여 액터 추가: %s (진영: %s)"), *GetNameSafe(NewCombatant), *UEnum::GetValueAsString(CombatantData->GetFaction()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 유효한 UCharacterBase 데이터를 가지지 않은 액터는 전투에 추가할 수 없습니다: %s"), *GetNameSafe(NewCombatant));
		}
	}
}

