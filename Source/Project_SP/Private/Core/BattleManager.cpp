#include "Core/BattleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/CombatPawn.h"
#include "Combat/BattleTurnComponent.h"
#include "Combat/CharacterStatsComponent.h"
#include "Combat/StatusEffectComponent.h"
#include "Event/GameEventComponent.h"
#include "Data/MonsterData.h"
#include "Core/MyGameInstance.h"
#include "Combat/MonsterCharacter.h" 


ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentTurnCharacter = nullptr;
	SetCurrentBattleState(EBattleState::Setup);
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

void ABattleManager::SetCurrentBattleState(EBattleState NewState)
{
	if (CurrentBattleState != NewState)
	{
		CurrentBattleState = NewState;
		OnBattleStateChanged.Broadcast(NewState); // 상태 변경 시 브로드캐스트
	}
}

void ABattleManager::StartBattle()
{
    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->IsValidLowLevel())
        {
            Combatant->Destroy();
        }
    }
    AllCombatants.Empty();

    // 1. 플레이어 전투원 추가 및 이벤트 바인딩
    if (APawn* PlayerPawnBase = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        if (ACombatPawn* PlayerCombatant = Cast<ACombatPawn>(PlayerPawnBase))
        {
            AllCombatants.Add(PlayerCombatant);
            if (PlayerCombatant->GameEventComponent)
            {
                PlayerCombatant->GameEventComponent->OnDamageReceived.AddDynamic(this, &ABattleManager::HandleCombatantDamageReceived);
                PlayerCombatant->GameEventComponent->OnHealthChanged.AddDynamic(this, &ABattleManager::HandleCombatantHealthChanged);
                PlayerCombatant->GameEventComponent->OnActionExecutionFinished.AddDynamic(this, &ABattleManager::HandleCombatantActionFinished);
                PlayerCombatant->GameEventComponent->OnTurnStarted.AddDynamic(this, &ABattleManager::HandleCombatantTurnStarted);
                PlayerCombatant->GameEventComponent->OnTurnEnded.AddDynamic(this, &ABattleManager::HandleCombatantTurnEnded);
            }
        }
        else { UE_LOG(LogTemp, Error, TEXT("Player Pawn is not ACombatPawn derived!")); return; }
    }
    else { UE_LOG(LogTemp, Error, TEXT("Player Pawn not found for battle!")); return; }

    // 2. 몬스터 스폰 및 이벤트 바인딩
    UMyGameInstance* MyGameInstance = GetGameInstance<UMyGameInstance>();
    if (MyGameInstance && MyGameInstance->PendingMonsterGroup)
    {
        TArray<FMonsterData> MonstersToSpawn = MyGameInstance->PendingMonsterGroup->GetAllMonsterDataInGroup();
        FVector SpawnLocationBase = GetActorLocation() + FVector(500.0f, 0.0f, 0.0f); // 몬스터 시작 스폰 위치
        float MonsterSpacing = 150.0f; // 몬스터 간 간격

        for (int32 i = 0; i < MonstersToSpawn.Num(); ++i)
        {
            const FMonsterData& MonsterData = MonstersToSpawn[i];
            if (MonsterData.MonsterClass)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                FVector SpawnLocation = SpawnLocationBase + FVector(0.0f, i * MonsterSpacing, 0.0f);
                AMonsterCharacter* SpawnedMonster = GetWorld()->SpawnActor<AMonsterCharacter>(MonsterData.MonsterClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

                if (SpawnedMonster)
                {
                    // 스탯 컴포넌트 초기화
                    if (UCharacterStatsComponent* StatsComp = SpawnedMonster->GetStatsComponent())
                    {
                        StatsComp->CharacterStatsDataTable = CharacterStatsDataTable;
                        StatsComp->RowName = MonsterData.CharacterStatsRowName;
                        StatsComp->InitializeStatsFromDataTable();
                    }
                    // 행동 테이블 및 ID 설정
                    SpawnedMonster->AvailableActionsDataTable = ActionsDataTable;
                    SpawnedMonster->MyActionIDs = MonsterData.AvailableActionIDs;
                    SpawnedMonster->SetFaction(EFaction::Enemy); // 몬스터 팩션 설정
                    AllCombatants.Add(SpawnedMonster); // 전투원 목록에 추가

                    // 몬스터 이벤트 바인딩
                    if (SpawnedMonster->GameEventComponent)
                    {
                        SpawnedMonster->GameEventComponent->OnDamageReceived.AddDynamic(this, &ABattleManager::HandleCombatantDamageReceived);
                        SpawnedMonster->GameEventComponent->OnHealthChanged.AddDynamic(this, &ABattleManager::HandleCombatantHealthChanged);
                        SpawnedMonster->GameEventComponent->OnActionExecutionFinished.AddDynamic(this, &ABattleManager::HandleCombatantActionFinished);
                        SpawnedMonster->GameEventComponent->OnTurnStarted.AddDynamic(this, &ABattleManager::HandleCombatantTurnStarted);
                        SpawnedMonster->GameEventComponent->OnTurnEnded.AddDynamic(this, &ABattleManager::HandleCombatantTurnEnded);
                    }
                }
            }
        }
    }

    // 3. 전투 시작 상태로 전환
    SetCurrentBattleState(EBattleState::InProgress); // 전투 관리자는 현재 상태만 변경
    OnTurnOrderChanged.Broadcast(); // UI 업데이트용 (초기 턴 순서 표시)
}

void ABattleManager::EndBattle()
{
    SetCurrentBattleState(EBattleState::Ended); // 전투 종료 상태로 변경
    UE_LOG(LogTemp, Warning, TEXT("Battle Ended!"));

    UMyGameInstance* MyGameInstance = GetGameInstance<UMyGameInstance>();
    if (MyGameInstance)
    {
        MyGameInstance->ReturnToFieldTransition();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EndBattle: MyGameInstance not found!"));
    }
}

void ABattleManager::ProcessTurn()
{
    if (CheckBattleEndConditions()) // 전투 종료 조건 확인
    {
        EndBattle(); // 전투 종료
        return;
    }

    if (CurrentBattleState == EBattleState::InProgress) // InProgress 상태일 때만 새로운 턴 탐색
    {
        TArray<ACombatPawn*> Ready = GetReadyCombatants(); // 턴을 잡을 준비가 된 캐릭터 목록
        if (Ready.Num() > 0)
        {
            // 정렬 기준에 따라 가장 먼저 턴을 잡을 캐릭터 선택
            Ready.Sort([](const ACombatPawn& A, const ACombatPawn& B) {
                return ABattleManager::CombatantSortPredicate(&A, &B);
                });
            InitiateTurnFor(Ready[0]); // 해당 캐릭터에게 턴 부여
        }
        else // 아무도 턴을 잡을 준비가 되지 않았으면, ActionValue를 증가시켜 다음 턴을 기다립니다.
        {
            float WaitTime = GetMinTimeToNextTurn();
            if (WaitTime <= 0.001f) WaitTime = 0.001f; // 무한 루프 방지 및 최소 시간 보장

            GlobalTime += WaitTime; // 전역 시간 증가
            AdvanceAllActionValues(WaitTime); // 모든 전투원의 ActionValue 증가
        }
    }
}

void ABattleManager::InitiateTurnFor(ACombatPawn* Target)
{
    // 타겟의 유효성 검사 (사망했거나 유효하지 않은 경우 턴 건너뜀)
    if (!Target || !Target->IsValidLowLevel() || !Target->GetStatsComponent() || Target->GetStatsComponent()->GetCurrentHealth() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to initiate turn for invalid or defeated target. Skipping."));
        return;
    }

    CurrentTurnCharacter = Target; // 현재 턴 캐릭터 설정

    //턴 시작 시 상태 이상 효과 처리
    Target->StatusEffectComponent->OnTurnStarted();

    if (auto Turn = Target->GetBattleTurnComponent())
    {
        Turn->StartTurn(); // 턴 컴포넌트에 턴 시작 알림 (ActionValue 리셋 등)
    }

    EFaction TargetFaction = Target->GetFaction();
    // 턴을 받은 캐릭터의 진영에 따라 BattleManager의 상태 변경
    // 플레이어 입력 제어는 BattleStateChanged 이벤트를 구독하는 PlayerController에서 처리합니다.
    if (TargetFaction == EFaction::Player)
    {
        SetCurrentBattleState(EBattleState::PlayerTurn); // 플레이어의 행동 선택을 기다리는 상태
    }
    else // Enemy
    {
        SetCurrentBattleState(EBattleState::EnemyTurn); // 몬스터 AI가 행동을 결정하는 상태
    }

    // 턴을 받은 캐릭터의 UGameEventComponent를 통해 턴 시작 이벤트 브로드캐스트
    // 해당 캐릭터(플레이어/몬스터)는 이 이벤트를 받아 각자 행동 로직을 시작함
    if (Target->GameEventComponent)
    {
        Target->GameEventComponent->BroadcastTurnStarted(Target);
    }

    // UI에 새로운 턴 순서를 반영하도록 알림
    OnTurnOrderChanged.Broadcast();
}

void ABattleManager::EndTurn()
{
    UE_LOG(LogTemp, Log, TEXT("EndTurn_Implementation called for %s"), CurrentTurnCharacter ? *CurrentTurnCharacter->GetName() : TEXT("N/A"));

    // 현재 턴 캐릭터의 턴 종료 처리 (ActionValue 리셋, bIsMyTurn = false 등)
    if (CurrentTurnCharacter && CurrentTurnCharacter->GetBattleTurnComponent())
    {
        CurrentTurnCharacter->GetBattleTurnComponent()->EndTurn();
    }

    // 현재 턴 캐릭터 초기화
    CurrentTurnCharacter = nullptr;
    // 상태를 InProgress로 변경하여 ProcessTurn이 다음 턴을 탐색하도록 함
    SetCurrentBattleState(EBattleState::InProgress);

    // UI에 턴이 완전히 넘어갔음을 알림 (턴 순서 업데이트)
    OnTurnOrderChanged.Broadcast();
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
	TArray<ACombatPawn*> ReadyCombatants;
	for (ACombatPawn* Combatant : AllCombatants)
	{
		if (Combatant && Combatant->IsValidLowLevel() &&
			Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0 &&
			Combatant != CurrentTurnCharacter && Combatant->GetBattleTurnComponent() && Combatant->GetBattleTurnComponent()->IsReadyForTurn())
		{
			ReadyCombatants.Add(Combatant);
		}
	}
	return ReadyCombatants;
}

float ABattleManager::GetMinTimeToNextTurn() const
{
    float MinTime = MAX_FLT;
    bool bFoundAnyReady = false;

    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->IsValidLowLevel() &&
            Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0 &&
            Combatant->GetBattleTurnComponent() && !Combatant->GetBattleTurnComponent()->GetIsMyTurn())
        {
            float TimeToAct = Combatant->GetBattleTurnComponent()->GetTimeLeftToAct();
            if (TimeToAct < MinTime)
            {
                MinTime = TimeToAct;
                bFoundAnyReady = true;
            }
        }
    }
    return bFoundAnyReady ? MinTime : 0.0f;
}

void ABattleManager::AdvanceAllActionValues(float DeltaTime)
{
    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->IsValidLowLevel() &&
            Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0 &&
            Combatant->GetBattleTurnComponent() && !Combatant->GetBattleTurnComponent()->GetIsMyTurn())
        {
            Combatant->GetBattleTurnComponent()->AdvanceActionValue(DeltaTime);
        }
    }
}



bool ABattleManager::CheckBattleEndConditions() const
{
    int32 PlayerCount = 0;
    int32 EnemyCount = 0;

    for (ACombatPawn* Combatant : AllCombatants)
    {
        if (Combatant && Combatant->IsValidLowLevel() && Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
        {
            if (Combatant->GetFaction() == EFaction::Player) PlayerCount++;
            else if (Combatant->GetFaction() == EFaction::Enemy) EnemyCount++;
        }
    }
    return PlayerCount == 0 || EnemyCount == 0;
}



bool ABattleManager::CombatantSortPredicate(const ACombatPawn* A, const ACombatPawn* B)
{
	float TimeA = A->GetBattleTurnComponent()->GetTimeLeftToAct();
	float TimeB = B->GetBattleTurnComponent()->GetTimeLeftToAct();
	if (!FMath::IsNearlyEqual(TimeA, TimeB)) return TimeA < TimeB;

	float SpeedA = A->GetStatsComponent()->GetMovementSpeed();
	float SpeedB = B->GetStatsComponent()->GetMovementSpeed();
	if (!FMath::IsNearlyEqual(SpeedA, SpeedB)) return SpeedA > SpeedB;

	if (A->GetFaction() != B->GetFaction()) return A->GetFaction() == EFaction::Player;

	return A->GetUniqueID() < B->GetUniqueID();
}

const TArray<ACombatPawn*>& ABattleManager::GetAllCombatants() const
{
	return AllCombatants;
}

ACombatPawn* ABattleManager::GetCurrentTurnCharacter() const
{
	return CurrentTurnCharacter;
}

// --- 이벤트 핸들러 구현 ---
void ABattleManager::HandleCombatantActionFinished(ACombatPawn* FinishedPawn)
{
    UE_LOG(LogTemp, Log, TEXT("[BM Event] Action finished for %s."), FinishedPawn ? *FinishedPawn->GetName() : TEXT("N/A"));
    if (FinishedPawn == CurrentTurnCharacter)
    {
        EndTurn();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BM Event] Action finished for %s, but they are not the current turn character (%s)."),
            FinishedPawn ? *FinishedPawn->GetName() : TEXT("N/A"),
            CurrentTurnCharacter ? *CurrentTurnCharacter->GetName() : TEXT("N/A"));
    }
}

void ABattleManager::HandleCombatantDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, ACombatPawn* InstigatorPawn, UDamageType* DamageType)
{
    UE_LOG(LogTemp, Log, TEXT("[BM Event] %s took %f damage from %s."), *DamagedPawn->GetName(), DamageAmount, InstigatorPawn ? *InstigatorPawn->GetName() : TEXT("Environment"));
}

void ABattleManager::HandleCombatantHealthChanged(ACombatPawn* CombatPawn, float CurrentHealth)
{
    UE_LOG(LogTemp, Log, TEXT("[BM Event] %s's health changed to %f"), *CombatPawn->GetName(), CurrentHealth);

    // 체력이 0 이하이고, 아직 사망 상태가 아닐 때만 실행
    if (CombatPawn && CombatPawn->GetStatsComponent()->GetCurrentHealth() <= 0 && CombatPawn->GetCombatPawnState() != ECombatPawnState::Defeated)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BM Event] %s has been defeated."), *CombatPawn->GetName());

        // 1. 캐릭터의 내부 상태를 '사망'으로 변경
        CombatPawn->InternalSetCombatPawnState(ECombatPawnState::Defeated);

        // 2. 시각적 처리 (사망 애니메이션, 소멸 이펙트 등) 호출 -> 블루프린트에서 구현
        CombatPawn->K2_OnDefeated();

        // 3. 충돌 비활성화 (더 이상 타겟팅되지 않도록)
        CombatPawn->SetActorEnableCollision(false);

        // 4. 현재 턴을 진행 중인 캐릭터가 사망했다면 즉시 턴 종료
        if (CombatPawn == CurrentTurnCharacter)
        {
            EndTurn();
        }

        // 5. 전투 종료 조건 확인 (모든 적 또는 모든 플레이어가 사망했는지)
        if (CheckBattleEndConditions())
        {
            EndBattle();
        }
    }
}

void ABattleManager::HandleCombatantTurnStarted(ACombatPawn* TurnPawn)
{
    
}

void ABattleManager::HandleCombatantTurnEnded(ACombatPawn* TurnPawn)
{
    UE_LOG(LogTemp, Log, TEXT("[BM Event] %s's turn has ended."), TurnPawn ? *TurnPawn->GetName() : TEXT("N/A"));
}



