#include "Combat/CombatPawn.h"
#include "Combat/CharacterStatsComponent.h"
#include "Combat/BattleTurnComponent.h"
#include "Combat/StatusEffectComponent.h"
#include "Data/ActionData.h"
#include "Event/GameEventComponent.h"
#include "Combat/GameAction.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BattleManager.h"

ACombatPawn::ACombatPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    StatsComponent = CreateDefaultSubobject<UCharacterStatsComponent>(TEXT("StatsComponent"));
    BattleTurnComponent = CreateDefaultSubobject<UBattleTurnComponent>(TEXT("BattleTurnComponent"));
    GameEventComponent = CreateDefaultSubobject<UGameEventComponent>(TEXT("GameEventComponent"));
    StatusEffectComponent = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComponent"));

    InternalSetCombatPawnState(ECombatPawnState::Idle); // 생성자에서 초기 상태 설정 (Internal 함수 사용)
}

void ACombatPawn::BeginPlay()
{
    Super::BeginPlay();
    // 전투 시작 시 초기 체력 설정 및 HealthChanged 이벤트 브로드캐스트 (예시)
    if (StatsComponent && GameEventComponent)
    {
        GameEventComponent->BroadcastHealthChanged(this, StatsComponent->GetCurrentHealth());
    }
}

// --- 캐릭터 상태 변경 함수 (내부 전용) ---
void ACombatPawn::InternalSetCombatPawnState(ECombatPawnState NewState)
{
    if (CurrentPawnState != NewState)
    {
        CurrentPawnState = NewState;
        // 상태 변경 시 외부로 이벤트 브로드캐스트 (예: UI 업데이트, AI 로직 변경 등)
        if (GameEventComponent)
        {
            GameEventComponent->BroadcastCombatPawnStateChanged(this, CurrentPawnState);
        }
    }
}


FActionData ACombatPawn::GetActionDataByID(FName ActionID) const
{
    if (AvailableActionsDataTable)
    {
        FActionData* FoundData = AvailableActionsDataTable->FindRow<FActionData>(ActionID, TEXT("Looking for Action Data"));
        if (FoundData) return *FoundData;
    }
    UE_LOG(LogTemp, Warning, TEXT("ActionData for ID '%s' not found for %s."), *ActionID.ToString(), *GetName());
    return FActionData();
}

TArray<FActionData> ACombatPawn::GetAllAvailableActions() const
{
    TArray<FActionData> ResultActions;
    if (!AvailableActionsDataTable) return ResultActions;
    for (const FName& ActionID : MyActionIDs)
    {
        FActionData* FoundData = AvailableActionsDataTable->FindRow<FActionData>(ActionID, TEXT("Getting All Available Actions"));
        if (FoundData) ResultActions.Add(*FoundData);
        else { UE_LOG(LogTemp, Warning, TEXT("ActionID '%s' in MyActionIDs not found in AvailableActionsDataTable for %s."), *ActionID.ToString(), *GetName()); }
    }
    return ResultActions;
}

void ACombatPawn::SelectAction(FName ActionID)
{
    // 이미 행동 수행 중이거나 패배한 상태라면 새로운 행동 선택 불가
    // ABattleManager나 APlayerController에서 턴 상태를 통제하여 SelectAction이 올바른 시점에 호출되도록 해야 함
    if (CurrentPawnState == ECombatPawnState::PerformingAction || CurrentPawnState == ECombatPawnState::Defeated)
    {
        return;
    }

    FActionData ActionData = GetActionDataByID(ActionID);

    if (!ActionData.GameActionClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActionData for '%s' has no valid GameActionClass assigned."), *ActionData.DisplayName.ToString());
        // 행동 선택 실패 (유효한 클래스 없음) -> 즉시 턴 종료 신호 (BattleManager가 다음 턴으로 넘어가도록)
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    ActiveActionInstance = NewObject<UGameAction>(this, ActionData.GameActionClass); // ActiveActionInstance 저장
    if (!ActiveActionInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create ActionInstance for %s."), *ActionData.DisplayName.ToString());
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    // 코스트 체크 (SelectAction 단계에서 코스트 체크를 수행하고 부족하면 바로 실패 처리)
    if (!ActiveActionInstance->HasEnoughCost(this, ActionData))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s does not have enough cost for action %s. Action cancelled."), *GetName(), *ActionData.DisplayName.ToString());
        ActiveActionInstance = nullptr; // 인스턴스 해제
        // 행동 취소 -> 즉시 턴 종료 신호 (BattleManager가 다음 턴으로 넘어가도록)
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    SelectedActionID = ActionID; // 선택된 ActionID 저장

    InternalSetCombatPawnState(ECombatPawnState::SelectingTarget);
}

void ACombatPawn::ExecuteConfirmedAction(FName ActionID, const TArray<ACombatPawn*>& ConfirmedTargets)
{
    UE_LOG(LogTemp, Log, TEXT("Execute 실행됨"));
    // 유효성 체크 및 상태 확인
    if (SelectedActionID != ActionID || !ActiveActionInstance || CurrentPawnState != ECombatPawnState::SelectingTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("유효성 체크 실패함"));
        // 비정상적인 호출이므로 BattleManager에게 턴을 넘겨 다음 턴으로 넘어가게 함
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        InternalSetCombatPawnState(ECombatPawnState::Idle); // 상태 초기화
        SelectedActionID = NAME_None;
        ActiveActionInstance = nullptr;
        return;
    }

    FActionData ActionData = GetActionDataByID(ActionID); // 최신 ActionData 가져오기

    InternalSetCombatPawnState(ECombatPawnState::PerformingAction); // 행동 수행 상태로 전환

    // UGameAction 인스턴스의 ExecuteAction 호출
    if (ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass())))
    {
        // UGameAction의 ExecuteAction 내에서 ABattleManager를 찾아 모든 전투원을 가져오도록 하였으므로, ABattleManager는 인자로 전달
        // ConfirmedTargets는 UGameAction::ExecuteAction에서 최종적으로 사용될 타겟 리스트
        ActiveActionInstance->ExecuteAction(this, ActionData, BattleManager, ConfirmedTargets.Num() > 0 ? ConfirmedTargets[0] : nullptr, ConfirmedTargets);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ABattleManager not found for executing action %s. Action cancelled."), *ActionID.ToString());
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this); // 행동을 실행할 수 없으므로 즉시 턴 종료 신호
        InternalSetCombatPawnState(ECombatPawnState::Idle); // 상태 초기화
        SelectedActionID = NAME_None;
        ActiveActionInstance = nullptr;
        return;
    }

    // 4. 이벤트 브로드캐스트 (ActionData는 ActionInstance에서 사용하는 FActionData와 동일)
    if (GameEventComponent)
    {
        GameEventComponent->BroadcastActionPerformed(this, ActionData);
    }

    // 5. 시각적/청각적 연출을 블루프린트로 위임
    K2_ExecuteActionVisuals(ActionData, ConfirmedTargets.Num() > 0 ? ConfirmedTargets[0] : nullptr);

    // TODO: 연출이 끝나면 GameEventComponent->BroadcastActionExecutionFinished(this);를 호출해야 함.
    // 이는 K2_ExecuteActionVisuals의 블루프린트 구현 내부(애니메이션 끝나는 지점)에서 처리되어야 합니다.
    // ABattleManager가 ActionExecutionFinished 이벤트를 받아 턴을 종료할 것이므로, 여기서 ACombatPawn의 상태를 즉시 Idle로 바꾸지 않습니다.
    // ABattleManager가 턴을 종료하고 다음 턴으로 넘어가면 다시 Idle 상태로 돌아갈 것입니다.
}


float ACombatPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (StatsComponent)
    {
        // 1. 현재 체력에서 데미지만큼 감소
        float NewHealth = StatsComponent->GetCurrentHealth() - DamageAmount;
        StatsComponent->SetCurrentHealth(NewHealth);

        // 2. 체력 변경 이벤트 호출 (UI 업데이트 등)
        if (GameEventComponent)
        {
            GameEventComponent->BroadcastHealthChanged(this, NewHealth);
            GameEventComponent->BroadcastDamageReceived(this, DamageAmount, Cast<ACombatPawn>(DamageCauser), nullptr);
        }
    }

    return DamageAmount;
}

void ACombatPawn::ResolveAction()
{
}

// --- Faction 및 컴포넌트 Getter ---
EFaction ACombatPawn::GetFaction() const
{
    return CurrentFaction;
}

void ACombatPawn::SetFaction(EFaction NewFaction)
{
    CurrentFaction = NewFaction;
}

UCharacterStatsComponent* ACombatPawn::GetStatsComponent() const
{
    return StatsComponent;
}

UBattleTurnComponent* ACombatPawn::GetBattleTurnComponent() const
{
    return BattleTurnComponent;
}

void ACombatPawn::BroadcastParryWindowEvent(bool bIsWindowOpen)
{
    if (GameEventComponent)
    {
        // 현재 내가 선택해서 사용 중인 공격(SelectedActionID)의 속성 정보를 가져옴
        FActionData MyAttackData = GetActionDataByID(SelectedActionID);
        GameEventComponent->BroadcastParryWindowChanged(this, MyAttackData.DamageType, bIsWindowOpen);
    }
}