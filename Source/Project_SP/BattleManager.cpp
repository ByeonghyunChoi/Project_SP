// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleManager.h"
#include "MonsterCharacter.h" 
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

auto BattleCombatantSortPredicate = [](const ACombatPawn& A, const ACombatPawn& B, const ABattleManager& Manager) -> bool
	{
		UCharacterBase* DataA = A.GetCombatData();
		UCharacterBase* DataB = B.GetCombatData();

		// 1. 유효하지 않은 데이터 처리 (nullptr은 항상 뒤로 보냄)
		if (!DataA && !DataB) return A.GetUniqueID() < B.GetUniqueID();
		if (!DataA) return false;
		if (!DataB) return true;

		// GetTimeLeftToAct()가 낮은 것이 우선 (오름차순)
		float TimeLeftA = DataA->GetTimeLeftToAct();
		float TimeLeftB = DataB->GetTimeLeftToAct();

		if (TimeLeftA != TimeLeftB)
		{
			return TimeLeftA < TimeLeftB;
		}
		else // TimeLeft가 같다면 다음 규칙 적용
		{
			// 2. 속도 비교 (속도 높은 순)
			float SpeedA = DataA->GetStats().fSpeed;
			float SpeedB = DataB->GetStats().fSpeed;

			if (SpeedA != SpeedB)
			{
				return SpeedA > SpeedB;
			}
			else // 속도도 같다면 진영 규칙 적용
			{
				EFaction FactionA = DataA->GetFaction();
				EFaction FactionB = DataB->GetFaction();

				// 규칙 5-1: 아군 진영이 우선
				if (FactionA == EFaction::Player && FactionB != EFaction::Player) return true;
				if (FactionB == EFaction::Player && FactionA != EFaction::Player) return false;

				// 그 외의 경우 (같은 진영, 같은 속도, 같은 TimeLeft)
				// 안정적인 정렬을 위해 고유 ID나 포인터 주소를 사용
				return A.GetUniqueID() < B.GetUniqueID();
			}
		}
	};
//==============================================================================


ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = true; // Tick 함수 활성화
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

	// 전투가 진행 중이고, 현재 턴을 가진 캐릭터가 없을 때만 ProcessTurn 호출
	if (CurrentBattleState == EBattleState::InProgress && CurrentTurnCharacter == nullptr)
	{
		ProcessTurn(); // <-- 핵심!
	}
}

void ABattleManager::StartBattle(TArray<ACombatPawn*> InitialCombatants)
{
	UE_LOG(LogTemp, Log, TEXT("ABattleManager: StartBattle 호출됨."));
	GlobalTime = 0.0f;
	CurrentTurnCharacter = nullptr;
	AllCombatants.Empty(); // 기존 전투 참여자 초기화

	// 초기 전투 참여자들을 AllCombatants에 추가
	for (ACombatPawn* CombatantActor : InitialCombatants)
	{
		AddCombatant(CombatantActor);
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();
		if (CombatantData)
		{
			// 캐릭터 이름 설정 (UI용)
			CombatantData->SetCharacterName(CombatantActor->GetName());
			// fActionValue는 UCharacterBase 생성자에서 0으로 초기화됨
		}
	}

	CurrentBattleState = EBattleState::InProgress;
	UE_LOG(LogTemp, Log, TEXT("ABattleManager: 전투 시작 준비 완료. 상태: InProgress."));

	if (OnTurnOrderChanged.IsBound()) // <-- 이 조건문과 로그 추가
	{
		UE_LOG(LogTemp, Log, TEXT("ABattleManager: OnTurnOrderChanged 델리게이트에 바인딩된 함수가 있습니다."));
		OnTurnOrderChanged.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ABattleManager: OnTurnOrderChanged 델리게이트에 바인딩된 함수가 없습니다!"));
	}
	ProcessTurn(); // 전투 시작 후 첫 턴 진행 시도
}

void ABattleManager::EndBattle()
{
	CurrentBattleState = EBattleState::Ended;
	UE_LOG(LogTemp, Log, TEXT("ABattleManager: 전투 종료."));
	// TODO: 전투 결과 처리 (승리/패배, 보상, 맵 복귀 등)
}

void ABattleManager::ProcessTurn()
{
	// 전투 종료 조건 확인
	if (CheckBattleEndConditions())
	{
		EndBattle();
		return;
	}

	// 1. 현재 턴을 잡을 준비가 된 캐릭터가 있는지 확인
	TArray<ACombatPawn*> ReadyCombatants;
	const float TargetDistance = 10000.0f; // 목표 거리 상수

	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();
		if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0 && CombatantData->IsReadyForTurn())
		{
			ReadyCombatants.Add(CombatantActor);
		}
	}

	// 2. 턴을 잡을 캐릭터가 있다면 (점프할 필요 없이 바로 턴 부여)
	if (ReadyCombatants.Num() > 0)
	{
		ReadyCombatants.Sort([this](const ACombatPawn& A, const ACombatPawn& B) -> bool
			{
				return BattleCombatantSortPredicate(A, B, *this);
			});
		InitiateTurnFor(ReadyCombatants[0]); // 가장 우선순위 높은 캐릭터에게 턴 부여
		return; // 턴이 부여되었으므로, 이 ProcessTurn 호출은 여기서 종료. 다음 턴은 EndTurn에서 다시 ProcessTurn을 호출하여 진행.
	}

	// 3. 턴을 잡을 캐릭터가 없다면, 다음 턴까지의 시간을 계산하여 GlobalTime 점프
	float MinTimeToNextTurn = 99999.0f; // 다음 턴까지 필요한 최소 시간

	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();
		// 살아있고, 아직 턴을 잡을 준비가 안 되었고, 속도가 0보다 큰 캐릭터만 고려
		if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0 && !CombatantData->IsReadyForTurn() && CombatantData->GetStats().fSpeed > 0)
		{
			float TimeLeftForThisCombatant = CombatantData->GetTimeLeftToAct(); // 이 캐릭터가 턴을 잡기까지 남은 시간
			if (TimeLeftForThisCombatant < MinTimeToNextTurn)
			{
				MinTimeToNextTurn = TimeLeftForThisCombatant;
			}
		}
	}

	// 4. GlobalTime을 점프시키고 모든 캐릭터의 fActionValue를 업데이트
	if (MinTimeToNextTurn < 99999.0f && MinTimeToNextTurn > 0.0f)
	{
		GlobalTime += MinTimeToNextTurn; // GlobalTime을 다음 턴 시점까지 점프

		for (ACombatPawn* CombatantActor : AllCombatants)
		{
			UCharacterBase* CombatantData = CombatantActor->GetCombatData();
			if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0)
			{
				CombatantData->AdvanceActionValue(MinTimeToNextTurn); // <-- fActionValue를 MinTimeToNextTurn만큼 증가
			}
		}

		// 시간 점프 후, 다시 ProcessTurn을 호출하여 턴을 잡을 캐릭터가 있는지 확인
		ProcessTurn(); // 재귀 호출 (Stack Overflow 주의: 방어 로직 필요)
	}
	else
	{
		// 더 이상 턴을 잡을 캐릭터가 없거나 모두 죽은 경우
		UE_LOG(LogTemp, Warning, TEXT("ABattleManager: 더 이상 턴을 획득할 캐릭터가 없습니다 (모두 사망 또는 속도 0). 전투가 멈추거나 종료될 수 있습니다."));
		CurrentBattleState = EBattleState::Ended; // 전투 종료 (선택 사항)
	}
}

void ABattleManager::EndTurn()
{
	UE_LOG(LogTemp, Log, TEXT("ABattleManager: 현재 턴 종료. 다음 턴 준비 중..."));
	if (CurrentTurnCharacter && CurrentTurnCharacter->GetCombatData())
	{
		CurrentTurnCharacter->GetCombatData()->EndTurn(); // bIsMyTurn = false 설정
		// fActionValue는 UCharacterBase::StartTurn()에서 이미 0으로 초기화됨
	}
	CurrentTurnCharacter = nullptr; // 현재 턴 캐릭터 초기화
	CurrentBattleState = EBattleState::InProgress; // 턴 종료 후 다시 InProgress로 전환하여 시간 진행 재개

	OnTurnOrderChanged.Broadcast(); // UI 연동 시도
	ProcessTurn(); // 턴 종료 후 바로 다음 턴 진행 (재귀 호출)
}

void ABattleManager::AddCombatant(ACombatPawn* NewCombatant)
{
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


void ABattleManager::InitiateTurnFor(ACombatPawn* TargetCombatant)
{
	if (!TargetCombatant || !TargetCombatant->GetCombatData())
	{
		UE_LOG(LogTemp, Error, TEXT("ABattleManager: 유효하지 않은 타겟에게 턴 부여 시도."));
		ProcessTurn(); // 문제 발생 시 다음 턴 진행 (재귀 호출)
		return;
	}

	CurrentTurnCharacter = TargetCombatant;
	CurrentTurnCharacter->GetCombatData()->StartTurn(); // bIsMyTurn = true 설정 및 fActionValue = 0.0f

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
	// 이 함수는 FindAndInitiateNextTurn에서 직접 사용되지 않으며,
	// GetTurnOrderCombatants에서만 BattleCombatantSortPredicate를 통해 사용됩니다.
	// 따라서 이 함수의 구현은 GetTurnOrderCombatants의 람다와 동일합니다.
	CombatantsToProcess.Sort([this](const ACombatPawn& A, const ACombatPawn& B) -> bool
		{
			return BattleCombatantSortPredicate(A, B, *this);
		});
}

bool ABattleManager::CheckBattleEndConditions() const
{
	int32 PlayerSideCount = 0;
	int32 EnemySideCount = 0;

	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		if (CombatantActor && CombatantActor->IsValidLowLevel())
		{
			UCharacterBase* CombatantData = CombatantActor->GetCombatData(); // GetCombatData() 직접 호출

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