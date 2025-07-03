// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleManager.h"
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

void ABattleManager::SortReadyCombatants(TArray<UCharacterBase*>& ReadyCombatants)
{
	// 이전에 UCharacterBase에서 논의했던 턴 우선순위 정렬 로직을 여기에 구현합니다.
	// 1. 속도 (fSpeed) 내림차순
	// 2. 속도가 같을 경우, 아군(Player) 진영 우선
	// 3. 속도와 진영이 같을 경우, 턴 순서 결정 인덱스 (iTurnOrderIndex) 오름차순
	ReadyCombatants.Sort([](const UCharacterBase& A, const UCharacterBase& B) -> bool
		{
			// 1. 속도 (fSpeed) 내림차순 정렬
			if (A.GetStats().fSpeed != B.GetStats().fSpeed) // FCharacterStatsData의 fSpeed 사용
			{
				return A.GetStats().fSpeed > B.GetStats().fSpeed;
			}

			// 2. 속도가 같을 경우, 아군(Player) 진영 우선
			if (A.GetFaction() != B.GetFaction())
			{
			    return A.GetFaction() == EFaction::Player;
			}

			// 3. 속도와 진영이 같을 경우, 턴 순서 결정 인덱스 (iTurnOrderIndex) 오름차순
			return A.GetTurnOrderIndex() < B.GetTurnOrderIndex();
		});
}

bool ABattleManager::CheckBattleEndConditions()
{
	int32 PlayerSideCount = 0;
	int32 EnemySideCount = 0;

	for (UCharacterBase* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->IsValidLowLevel() && Combatant->GetStats().fCurrentHealth > 0)
		{
			//Combatant의 진영을 확인하여 PlayerSideCount 또는 EnemySideCount 증가
			if (Combatant->GetFaction() == EFaction::Player)
			{
			    PlayerSideCount++;
			}
			else if (Combatant->GetFaction() == EFaction::Enemy)
			{
			     EnemySideCount++;
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

// Called every frame
void ABattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentBattleState == EBattleState::InProgress ||
		CurrentBattleState == EBattleState::PlayerTurn ||
		CurrentBattleState == EBattleState::EnemyTurn)
	{
		GlobalTime += DeltaTime;

		// 모든 전투 참여자의 행동 게이지 업데이트
		for (UCharacterBase* Combatant : AllCombatants)
		{
			if (Combatant && Combatant->IsValidLowLevel() && Combatant->GetStats().fCurrentHealth > 0) // 유효성 및 생존 확인
			{
				Combatant->UpdateActionGauge(DeltaTime);
			}
		}

		// 현재 턴 캐릭터가 없거나, 현재 턴 캐릭터가 행동을 완료했다면 다음 턴을 처리합니다.
		// (CurrentTurnCharacter가 nullptr이거나, GetIsMyTurn()이 false라면)
		if (!CurrentTurnCharacter || !CurrentTurnCharacter->GetIsMyTurn())
		{
			ProcessTurn();
		}
	}

}

void ABattleManager::StartBattle(TArray<UCharacterBase*> InitialCombatants)
{
	CurrentBattleState = EBattleState::Setup;
	GlobalTime = 0.0f;
	CurrentTurnCharacter = nullptr;
	AllCombatants.Empty();

	for (UCharacterBase* Combatant : InitialCombatants)
	{
		AddCombatant(Combatant);
	}

	CurrentBattleState = EBattleState::InProgress;
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
	TArray<UCharacterBase*> ReadyCombatants;
	for (UCharacterBase* Combatant : AllCombatants)
	{
		// 유효하고, 살아있고, 턴을 획득할 준비가 된 캐릭터만 추가
		if (Combatant && Combatant->IsValidLowLevel() && Combatant->GetStats().fCurrentHealth > 0 && Combatant->IsReadyForTurn())
		{
			ReadyCombatants.Add(Combatant);
		}
	}

	if (ReadyCombatants.Num() == 0)
	{
		return;
	}

	SortReadyCombatants(ReadyCombatants);

	CurrentTurnCharacter = ReadyCombatants[0];
	CurrentTurnCharacter->StartTurn();

	if (CurrentTurnCharacter->GetFaction() == EFaction::Player) // 플레이어 진영이라면
	{
		CurrentBattleState = EBattleState::PlayerTurn;

		// 모든 플레이어 캐릭터를 찾아 해당 UCharacterBase 인스턴스를 가진 플레이어에게 턴을 알립니다.
		// 게임에 플레이어 캐릭터가 하나라는 가정 하에 GetPlayerCharacter(0)을 사용합니다.
		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		// PlayerChar가 유효하고, 해당 플레이어 캐릭터의 BasicStats가 현재 턴 캐릭터와 동일한 UCharacterBase 인스턴스라면
		if (PlayerChar && PlayerChar->BasicStats == CurrentTurnCharacter)
		{
			// APlayerCharacter에 UBattleModeComponent* BattleModeComp;가 선언되어 있어야 합니다.
			if (PlayerChar->BattleModeComp) // BattleModeComp가 유효한지 확인
			{
				PlayerChar->BattleModeComp->ReceiveTurn(); // BattleModeComponent의 ReceiveTurn 함수 호출
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter에 BattleModeComp가 유효하지 않습니다."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("현재 턴 캐릭터가 플레이어 진영이지만, 해당 플레이어 캐릭터 인스턴스를 찾을 수 없거나 BasicStats가 일치하지 않습니다."));
		}
	}
	else // 몬스터나 NPC (Enemy 진영이라면)
	{
		CurrentBattleState = EBattleState::EnemyTurn;
		UE_LOG(LogTemp, Log, TEXT("몬스터 턴 시작! 행동 결정 중..."));
		CurrentTurnCharacter->DecideAction(); // 몬스터/NPC의 AI 행동 결정 및 수행
		// UMonsterBase::DecideAction() (또는 그 파생 클래스) 내부에서 반드시 EndTurn()을 호출해야 합니다.
	}

}

void ABattleManager::AddCombatant(UCharacterBase* NewCombatant)
{
	if (NewCombatant && NewCombatant->IsValidLowLevel())
	{
		AllCombatants.AddUnique(NewCombatant); // 중복 추가 방지
		UE_LOG(LogTemp, Log, TEXT("전투 참여자 추가: %s"), *GetNameSafe(NewCombatant));
	}
}

