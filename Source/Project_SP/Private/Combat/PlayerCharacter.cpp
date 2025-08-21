#include "Combat/PlayerCharacter.h"
#include "Core/BattleManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Event/GameEventComponent.h"
#include "Equipment/WeaponSystemComponent.h"
#include "GameMode/FieldModeComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
    FieldModeComp = CreateDefaultSubobject<UFieldModeComponent>(TEXT("FieldModeComponent"));
    WeaponSystemComponent = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystemComponent"));

    FieldModeComp->SetComponentTickEnabled(false);

    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    CurrentlySelectedTarget = nullptr;
}

void APlayerCharacter::EnterFieldMode()
{
    // 필드 모드 컴포넌트 활성화
    if (FieldModeComp)
    {
        FieldModeComp->SetComponentTickEnabled(true);
        UE_LOG(LogTemp, Log, TEXT("플레이어 필드 모드 진입."));
    }
}

void APlayerCharacter::EnterBattleMode()
{
    // 전투 진입 시 필드 모드 컴포넌트 비활성화
    if (FieldModeComp)
    {
        FieldModeComp->SetComponentTickEnabled(false);
        UE_LOG(LogTemp, Log, TEXT("필드 모드 비활성화."));
    }
}

// -- - 이벤트 핸들러 구현-- -
void APlayerCharacter::HandlePlayerTurnStarted(ACombatPawn* TurnPawn)
{
    if (TurnPawn == this) // 내 턴이 시작되었을 때
    {
        UE_LOG(LogTemp, Log, TEXT("%s (Player) Turn Started. Awaiting Action Selection."), *GetName());
        // TODO: 플레이어 턴 UI (행동 선택 메뉴) 활성화
        // ACombatPawn의 InternalSetCombatPawnState를 통해 ECombatPawnState::SelectingAction으로 상태 변경
        InternalSetCombatPawnState(ECombatPawnState::SelectingAction);

        ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
        if (BattleManager)
        {
            for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
            {
                if (Combatant && Combatant->GetFaction() == EFaction::Enemy && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
                {
                    CurrentlySelectedTarget = Combatant; // 첫 번째 적을 현재 타겟으로 저장
                    if (GameEventComponent)
                    {
                        // 이 줄을 추가하여 UI에 신호를 보냅니다.
                        GameEventComponent->BroadcastTargetChanged(CurrentlySelectedTarget);
                    }
                    break;
                }
            }
        }
    }
}

void APlayerCharacter::HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState)
{
    if (Pawn == this) // 내 Pawn 상태가 변경되었을 때
    {
        // TODO: 상태에 따른 UI 피드백 (예: SelectingTarget 상태 시 타겟 선택 UI 활성화)
    }
}

// --- 플레이어 행동/타겟 선택 로직 ---
void APlayerCharacter::PlayerSelectAction(FName ActionID)
{
    // 이미 행동 중이거나 패배했다면 선택 불가
    if (GetCombatPawnState() == ECombatPawnState::PerformingAction || GetCombatPawnState() == ECombatPawnState::Defeated) return;

    // 새로운 행동 버튼을 눌렀을 경우
    SelectedActionID = ActionID;
    UE_LOG(LogTemp, Log, TEXT("Action '%s' Selected."), *ActionID.ToString());

    FActionData ActionData = GetActionDataByID(ActionID);
    if (ActionData.GameActionClass)
    {
        ActiveActionInstance = NewObject<UGameAction>(this, ActionData.GameActionClass);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Action %s has no GameActionClass assigned!"), *ActionID.ToString());
        ActiveActionInstance = nullptr; // 유효하지 않으면 null로 초기화
    }
}

void APlayerCharacter::PlayerConfirmSelectedAction()
{
    // 선택된 액션이나 활성화된 인스턴스가 없으면 실행 불가
    if (SelectedActionID == NAME_None || !ActiveActionInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("No action selected to confirm."));
        return;
    }

    // 코스트가 부족하면 실행 불가
    if (!ActiveActionInstance->HasEnoughCost(this, GetActionDataByID(SelectedActionID)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough cost for action '%s'."), *SelectedActionID.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Action '%s' Confirmed!"), *SelectedActionID.ToString());

    TArray<ACombatPawn*> ConfirmedTargets;
    FActionData ActionData = GetActionDataByID(SelectedActionID);

    // 전체 공격이 아니면 현재 선택된 타겟을 사용
    if (ActionData.TargetingType != ETargetingType::All && CurrentlySelectedTarget)
    {
        ConfirmedTargets.Add(CurrentlySelectedTarget);
    }

    // 유효한 타겟이 있거나 전체 공격일 경우에만 실행
    if (ConfirmedTargets.Num() > 0 || ActionData.TargetingType == ETargetingType::All)
    {
        // 부모의 ExecuteConfirmedAction을 호출하여 실제 행동 실행
        Super::ExecuteConfirmedAction(SelectedActionID, ConfirmedTargets);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid target selected for action '%s'."), *SelectedActionID.ToString());
    }
}


void APlayerCharacter::PlayerSwitchTarget(bool bSwitchToNext)
{
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager || GetCombatPawnState() == ECombatPawnState::PerformingAction) return;

    // 현재 살아있는 모든 적 목록을 가져옵니다.
    TArray<ACombatPawn*> EnemyList;
    for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
    {
        if (Combatant && Combatant->GetFaction() == EFaction::Enemy && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
        {
            EnemyList.Add(Combatant);
        }
    }

    if (EnemyList.Num() == 0) return; // 적이 없으면 종료

    int32 CurrentIndex = EnemyList.Find(CurrentlySelectedTarget);
    int32 NewIndex = bSwitchToNext ? CurrentIndex + 1 : CurrentIndex - 1;

    // 인덱스 범위 처리 (배열의 처음과 끝을 순환)
    if (NewIndex >= EnemyList.Num()) NewIndex = 0;
    if (NewIndex < 0) NewIndex = EnemyList.Num() - 1;

    CurrentlySelectedTarget = EnemyList[NewIndex];
    if (GameEventComponent)
    {
        // 이 줄을 추가하여 UI에 신호를 보냅니다.
        GameEventComponent->BroadcastTargetChanged(CurrentlySelectedTarget);
    }
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    SetFaction(EFaction::Player);
    if (GameEventComponent)
    {
        GameEventComponent->OnTurnStarted.AddDynamic(this, &APlayerCharacter::HandlePlayerTurnStarted);
        GameEventComponent->OnCombatPawnStateChanged.AddDynamic(this, &APlayerCharacter::HandleMyPawnStateChanged);
    }
}