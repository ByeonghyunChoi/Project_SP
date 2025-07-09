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
		if (!DataA && !DataB) return A.GetUniqueID() < B.GetUniqueID(); // 둘 다 nullptr이면 ID로 정렬
		if (!DataA) return false; // A만 nullptr이면 A가 뒤로 (B가 먼저)
		if (!DataB) return true;  // B만 nullptr이면 B가 뒤로 (A가 먼저)

		// GetTimeLeftToAct()가 낮은 것이 우선 (오름차순)
		float TimeLeftA = DataA->GetTimeLeftToAct();
		float TimeLeftB = DataB->GetTimeLeftToAct();

		if (TimeLeftA != TimeLeftB)
		{
			return TimeLeftA < TimeLeftB; // <-- TimeLeft 낮은 순
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

				// 적군 진영 내에서는 아무나 턴을 잡도록 규칙 5-2 제거

				// 그 외의 경우 (같은 진영, 같은 속도, 같은 TimeLeft)
				// 안정적인 정렬을 위해 고유 ID나 포인터 주소를 사용
				return A.GetUniqueID() < B.GetUniqueID(); // 고유 ID가 작은 순 (안정적 정렬)
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

	// 전투가 진행 중이고, 현재 턴을 가진 캐릭터가 없을 때만 GlobalTime을 증가시키고 턴 탐색
	// (턴 중에는 GlobalTime 증가 및 fActionValue 업데이트를 멈춤)
	if (CurrentBattleState == EBattleState::InProgress && CurrentTurnCharacter == nullptr)
	{
		GlobalTime += DeltaTime; // GlobalTime을 매 틱마다 증가

		// 모든 캐릭터의 fActionValue를 업데이트 (달린 거리 증가)
		for (ACombatPawn* CombatantActor : AllCombatants)
		{
			UCharacterBase* CombatantData = CombatantActor->GetCombatData();
			if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0)
			{
				CombatantData->UpdateActionValue(DeltaTime * 500.0f); // fActionValue 증가
			}
		}

		FindAndInitiateNextTurn(); // fActionValue가 10000에 도달했는지 확인하고 턴 진행
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

	// OnTurnOrderChanged.Broadcast(); // 이제 PredictOrder가 관리하므로 호출 안함
	FindAndInitiateNextTurn(); // 전투 시작 후 첫 턴 진행 시도 (바로 턴을 잡을 캐릭터가 있을 수 있음)
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

	// CTB 핵심: fActionValue가 10000에 도달했는지 확인하고 턴 진행
	FindAndInitiateNextTurn();
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

	// OnTurnOrderChanged.Broadcast(); // 이제 PredictOrder가 관리하므로 호출 안함
	FindAndInitiateNextTurn(); // 턴 종료 후 바로 다음 턴 진행 시도
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

// CTB 핵심: fActionValue가 10000에 도달한 캐릭터를 찾아 턴을 부여
void ABattleManager::FindAndInitiateNextTurn()
{
	// 1. 턴을 획득할 준비가 된 캐릭터 목록 생성 (fActionValue가 10000 이상인 캐릭터)
	TArray<ACombatPawn*> ReadyCombatants;
	const float TargetDistance = 10000.0f; // 목표 거리 상수
	for (ACombatPawn* CombatantActor : AllCombatants)
	{
		UCharacterBase* CombatantData = CombatantActor->GetCombatData();
		// 살아있고, fActionValue가 10000에 도달했으면 턴 준비 완료
		if (CombatantData && CombatantData->GetStats().fCurrentHealth > 0 && CombatantData->IsReadyForTurn())
		{
			ReadyCombatants.Add(CombatantActor);
		}
	}

	// 2. 턴을 획득할 캐릭터가 있다면 바로 턴 부여
	if (ReadyCombatants.Num() > 0)
	{
		// 정렬 로직 람다 사용 (BattleCombatantSortPredicate)
		ReadyCombatants.Sort([this](const ACombatPawn& A, const ACombatPawn& B) -> bool
			{
				return BattleCombatantSortPredicate(A, B, *this);
			});
		InitiateTurnFor(ReadyCombatants[0]); // 가장 우선순위 높은 캐릭터에게 턴 부여
	}
	// 턴을 획득할 캐릭터가 없다면, GlobalTime은 Tick에서 계속 진행됩니다.
}

void ABattleManager::InitiateTurnFor(ACombatPawn* TargetCombatant)
{
	if (!TargetCombatant || !TargetCombatant->GetCombatData())
	{
		UE_LOG(LogTemp, Error, TEXT("ABattleManager: 유효하지 않은 타겟에게 턴 부여 시도."));
		// 문제 발생 시 (예: 대상이 죽었거나 데이터 없음) 다음 턴 진행 시도
		ProcessTurn();
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