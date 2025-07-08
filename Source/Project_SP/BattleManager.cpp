// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleManager.h"
#include "MonsterCharacter.h" 
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

auto BattleCombatantSortPredicate = [](const ACombatPawn& A, const ACombatPawn& B, const ABattleManager& Manager) -> bool
	{
		UCharacterBase* DataA = A.GetCombatData();
		UCharacterBase* DataB = B.GetCombatData();
		if (!DataA || !DataB)
		{
			return A.GetUniqueID() < B.GetUniqueID();
		}
		if (!DataA) return false;
		if (!DataB) return true;

		float ActionGaugeA = DataA->GetActionGauge();
		float ActionGaugeB = DataB->GetActionGauge();

		// 1. 행동 게이지가 다르면 행동 게이지 높은 순 (기본)
		if (ActionGaugeA != ActionGaugeB)
		{
			return ActionGaugeA > ActionGaugeB;
		}
		else // 행동 게이지가 같다면 다음 규칙 적용
		{
			// 2. 속도 비교 (기본)
			float SpeedA = DataA->GetStats().fSpeed;
			float SpeedB = DataB->GetStats().fSpeed;

			if (SpeedA != SpeedB)
			{
				return SpeedA > SpeedB; // 속도 높은 순
			}
			else // 행동 게이지와 속도가 같다면 진영 및 위치 규칙 적용
			{
				EFaction FactionA = DataA->GetFaction();
				EFaction FactionB = DataB->GetFaction();

				// 규칙: 아군 진영이 우선
				if (FactionA == EFaction::Player && FactionB != EFaction::Player) return true;
				if (FactionB == EFaction::Player && FactionA != EFaction::Player) return false;

				return A.GetUniqueID() < B.GetUniqueID();
			}
		}
	};

// Sets default values
ABattleManager::ABattleManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CurrentBattleState = EBattleState::Setup;
	GlobalTime = 0.0f;
	CurrentTurnCharacter = nullptr;
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();

}

void ABattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 전투가 진행 중이고, 현재 턴을 가진 캐릭터가 없을 때만 시간 진행 및 턴 탐색
	if (CurrentBattleState == EBattleState::InProgress && CurrentTurnCharacter == nullptr)
	{
		ProcessTurn(); // <-- 핵심 전투 로직 호출
	}
}

void ABattleManager::StartBattle(TArray<ACombatPawn*> InitialCombatants)
{
	UE_LOG(LogTemp, Log, TEXT("StartBattle: 함수 진입."));
	GlobalTime = 0.0f;
	CurrentTurnCharacter = nullptr;
	AllCombatants.Empty();

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
	AdvanceTimeAndFindNextTurn();
}

void ABattleManager::EndTurn()
{
	UE_LOG(LogTemp, Log, TEXT("ABattleManager: 현재 턴 종료. 다음 턴 준비 중..."));
	if (CurrentTurnCharacter && CurrentTurnCharacter->GetCombatData())
	{
		CurrentTurnCharacter->GetCombatData()->EndTurn(); // 현재 캐릭터의 턴 종료 플래그만 변경
	}
	CurrentTurnCharacter = nullptr; // 현재 턴 캐릭터 초기화
	CurrentBattleState = EBattleState::InProgress; // 턴 종료 후 다시 InProgress로 전환하여 시간 진행 재개
}


void ABattleManager::AddCombatant(ACombatPawn* NewCombatant)
{
	// ACombatPawn*가 유효하고, GetCombatData()를 통해 UCharacterBase*를 얻을 수 있을 때만 추가
	if (NewCombatant && NewCombatant->IsValidLowLevel())
	{
		UCharacterBase* CombatantData = NewCombatant->GetCombatData();
		if (CombatantData)
		{
			AllCombatants.AddUnique(NewCombatant); // 중복 추가 방지
			UE_LOG(LogTemp, Log, TEXT("ABattleManager: 전투 참여 액터 추가: %s (진영: %s)"), *GetNameSafe(NewCombatant), *UEnum::GetValueAsString(CombatantData->GetFaction()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 유효한 UCharacterBase 데이터를 가지지 않은 액터는 전투에 추가할 수 없습니다: %s"), *GetNameSafe(NewCombatant));
		}
	}
}

void ABattleManager::AdvanceTimeAndFindNextTurn()
{
	TArray<ACombatPawn*> ReadyCombatants;
	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();
		if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0 && CombatantData->IsReadyForTurn())
		{
			ReadyCombatants.Add(CombatantActor);
		}
	}

	// 2. 턴을 획득할 캐릭터가 있다면 바로 턴 부여
	if (ReadyCombatants.Num() > 0)
	{
		SortReadyCombatants(ReadyCombatants); // 규칙 5 적용
		CurrentTurnCharacter = ReadyCombatants[0];
		InitiateTurnFor(CurrentTurnCharacter);
		return; // 턴을 얻었으니 더 이상 시간 진행하지 않고 종료
	}

	// 3. 턴을 획득할 캐릭터가 없다면, 다음 턴 캐릭터가 100을 채우기까지 필요한 최소 시간을 계산하여 시간 진행
	float MinTimeToReach100 = 99999.0f; // 매우 큰 값으로 초기화
	bool bCanAdvanceTime = false; // 시간을 진행할 수 있는 캐릭터가 있는지

	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();
		if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0 && CombatantData->GetStats().fSpeed > 0)
		{
			bCanAdvanceTime = true;
			float CurrentGauge = CombatantData->GetActionGauge();
			float Speed = CombatantData->GetStats().fSpeed;

			// 100까지 남은 게이지
			float RemainingGauge = FMath::Max(0.0f, 100.0f - CurrentGauge); // 이미 100 이상이면 0으로 처리

			// 100까지 도달하는 데 필요한 시간 (게이지 / 속도)
			float TimeNeeded = RemainingGauge / Speed; // 계수 조절 필요 (예: / (Speed * 0.1f) )

			// 가장 먼저 100에 도달할 캐릭터의 시간 찾기
			if (TimeNeeded < MinTimeToReach100)
			{
				MinTimeToReach100 = TimeNeeded;
			}
		}
	}

	// MinTimeToReach100이 유효하다면, 모든 캐릭터의 행동 게이지를 그 시간만큼 증가
	if (bCanAdvanceTime && MinTimeToReach100 < 99999.0f && MinTimeToReach100 > 0.0f)
	{
		GlobalTime += MinTimeToReach100; // GlobalTime 증가
		for (ACombatPawn* CombatantActor : AllCombatants)
		{
			UCharacterBase* CombatantData = CombatantActor->GetCombatData();
			if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0)
			{
				// 모든 캐릭터의 행동 게이지를 동시에 증가
				CombatantData->UpdateActionGauge(MinTimeToReach100 * CombatantData->GetStats().fSpeed); // 계수 조절 필요
			}
		}
	}
	else
	{
		CurrentBattleState = EBattleState::Ended; // 전투 종료
	}
	
}

void ABattleManager::InitiateTurnFor(ACombatPawn* TargetCombatant)
{
	if (!TargetCombatant || !TargetCombatant->GetCombatData())
	{
		UE_LOG(LogTemp, Error, TEXT("ABattleManager: 유효하지 않은 타겟에게 턴 부여 시도."));
		ProcessTurn(); // 문제 발생 시 다음 턴 진행 시도
		return;
	}

	CurrentTurnCharacter = TargetCombatant;
	CurrentTurnCharacter->GetCombatData()->StartTurn(); // <-- UCharacterBase::StartTurn 호출 (행동 게이지 100% 차감)

	// 진영에 따라 턴 상태 변경 및 해당 컴포넌트 호출
	if (CurrentTurnCharacter->GetCombatData()->GetFaction() == EFaction::Player)
	{
		CurrentBattleState = EBattleState::PlayerTurn;
		UE_LOG(LogTemp, Log, TEXT("ABattleManager: 플레이어 턴 시작! (%s). 입력 대기 중..."), *GetNameSafe(CurrentTurnCharacter));

		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CurrentTurnCharacter);
		if (PlayerChar && PlayerChar->BattleModeComp)
		{
			PlayerChar->BattleModeComp->ReceiveTurn(); // 플레이어 행동 선택 대기
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 플레이어 턴이나, APlayerCharacter 또는 BattleModeComp가 유효하지 않습니다. 턴 종료 시도."));
			EndTurn(); // 문제 발생 시 턴 종료
		}
	}
	else // 몬스터나 NPC
	{
		CurrentBattleState = EBattleState::EnemyTurn;
		UE_LOG(LogTemp, Log, TEXT("ABattleManager: 몬스터 턴 시작! (%s). 행동 결정 중..."), *GetNameSafe(CurrentTurnCharacter));

		AMonsterCharacter* MonsterChar = Cast<AMonsterCharacter>(CurrentTurnCharacter);
		if (MonsterChar)
		{
			MonsterChar->PerformMonsterTurnAction(); // 몬스터 AI 행동 시작
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 몬스터 턴이나, AMonsterCharacter가 유효하지 않습니다. 턴 종료 시도."));
			EndTurn(); // 문제 발생 시 턴 종료
		}
	}
}

void ABattleManager::SortReadyCombatants(TArray<ACombatPawn*>& CombatantsToProcess) const
{
	CombatantsToProcess.Sort([this](const ACombatPawn& A, const ACombatPawn& B) -> bool
		{
			// BattleCombatantSortPredicate 람다를 호출
			return BattleCombatantSortPredicate(A, B, *this);
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

TArray<ACombatPawn*> ABattleManager::GetTurnOrderCombatants() const
{
	TArray<ACombatPawn*> SortedCombatants = AllCombatants;

	SortedCombatants.Sort([this](const ACombatPawn& A, const ACombatPawn& B) -> bool
		{
			// BattleCombatantSortPredicate 람다를 호출
			return BattleCombatantSortPredicate(A, B, *this);
		});
	return SortedCombatants;
}



