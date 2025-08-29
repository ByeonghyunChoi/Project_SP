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

    iCurrentEXP = 0;
    iNextLevelEXP = 100;
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
        InternalSetCombatPawnState(ECombatPawnState::SelectingAction);

        // 턴 시작 시 타겟 리스트를 비워줍니다.
        SelectedTargetList.Empty();
        if (GameEventComponent)
        {
            GameEventComponent->BroadcastTargetListChanged(SelectedTargetList);
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

void APlayerCharacter::LevelUp()
{
    iCurrentEXP -= iNextLevelEXP;

    int32 NewLevel = StatsComponent->GetCharacterLevel() + 1;

    // StatsComponent의 재계산 함수 호출
    StatsComponent->RecalculateStatsForLevelUp(NewLevel);
    // 체력은 최대로 회복
    StatsComponent->SetCurrentHealth(StatsComponent->GetMaxHealth());

    // 다음 레벨업에 필요한 경험치 재설정 (나중에 조정하실 수 있도록 그대로 둠)
    iNextLevelEXP = FMath::RoundToInt(iNextLevelEXP * 1.5f);
}

// --- 플레이어 행동/타겟 선택 로직 ---
void APlayerCharacter::PlayerSelectAction(FName ActionID)
{
    UE_LOG(LogTemp, Warning, TEXT("<<<<< 1. C++ PlayerSelectAction Called! >>>>>"));
    if (GetCombatPawnState() == ECombatPawnState::PerformingAction || GetCombatPawnState() == ECombatPawnState::Defeated) return;

    SelectedActionID = ActionID;
    UE_LOG(LogTemp, Log, TEXT("Action '%s' Selected."), *ActionID.ToString());

    FActionData ActionData = GetActionDataByID(ActionID);
    if (!ActionData.GameActionClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Action %s has no GameActionClass assigned!"), *ActionID.ToString());
        ActiveActionInstance = nullptr;
        return;
    }

    ActiveActionInstance = NewObject<UGameAction>(this, ActionData.GameActionClass);

    // 타겟 리스트 초기화
    SelectedTargetList.Empty();

    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager) return;

    TArray<ACombatPawn*> LivingEnemies;
    for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
    {
        if (Combatant && Combatant->GetFaction() == EFaction::Enemy && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
        {
            LivingEnemies.Add(Combatant);
        }
    }

    if (LivingEnemies.Num() == 0) return; // 타겟이 없으면 종료

    // 스킬의 타겟팅 타입에 따라 초기 타겟 설정
    switch (ActionData.TargetingType)
    {
    case ETargetingType::Single:
        SelectedTargetList.Add(LivingEnemies[0]);
        break;

    case ETargetingType::Dual:
        SelectedTargetList.Add(LivingEnemies[0]);
        if (LivingEnemies.Num() > 1)
        {
            SelectedTargetList.Add(LivingEnemies[1]);
        }
        break;

    case ETargetingType::All:
        SelectedTargetList = LivingEnemies;
        break;
    }

    InternalSetCombatPawnState(ECombatPawnState::SelectingTarget);

    // UI에 타겟 리스트가 변경되었음을 알림
    if (GameEventComponent)
    {
        GameEventComponent->BroadcastTargetListChanged(SelectedTargetList); 
    }
}

void APlayerCharacter::PlayerConfirmSelectedAction()
{
    if (SelectedActionID == NAME_None || !ActiveActionInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("No action selected to confirm."));
        return;
    }
    if (!ActiveActionInstance->HasEnoughCost(this, GetActionDataByID(SelectedActionID)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough cost for action '%s'."), *SelectedActionID.ToString());
        return;
    }

    // 유효한 타겟이 있을 경우에만 실행
    if (SelectedTargetList.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Action '%s' Confirmed! Targeting %d pawns."), *SelectedActionID.ToString(), SelectedTargetList.Num());
        // 부모의 ExecuteConfirmedAction을 호출하여 실제 행동 실행
        Super::ExecuteConfirmedAction(SelectedActionID, SelectedTargetList);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid target selected for action '%s'."), *SelectedActionID.ToString());
    }
}


void APlayerCharacter::PlayerSwitchTarget(bool bSwitchToNext)
{
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager || GetCombatPawnState() != ECombatPawnState::SelectingTarget) return;

    TArray<ACombatPawn*> LivingEnemies;
    for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
    {
        if (Combatant && Combatant->GetFaction() == EFaction::Enemy && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
        {
            LivingEnemies.Add(Combatant);
        }
    }

    if (LivingEnemies.Num() <= 1) return;

    FActionData ActionData = GetActionDataByID(SelectedActionID);

    int32 CurrentIndex = -1;
    if (SelectedTargetList.Num() > 0)
    {
        CurrentIndex = LivingEnemies.Find(SelectedTargetList[0]);
    }
    else // 만약 선택된 타겟이 없다면 0번을 기준으로 함
    {
        CurrentIndex = 0;
    }

    int32 NewIndex = bSwitchToNext ? CurrentIndex + 1 : CurrentIndex - 1;
    if (NewIndex >= LivingEnemies.Num()) NewIndex = 0;
    if (NewIndex < 0) NewIndex = LivingEnemies.Num() - 1;

    SelectedTargetList.Empty();

    // 스킬 타입에 따라 선택 리스트를 다시 채움
    switch (ActionData.TargetingType)
    {
    case ETargetingType::Single:
        SelectedTargetList.Add(LivingEnemies[NewIndex]);
        break;

    case ETargetingType::Dual:
    {
        SelectedTargetList.Add(LivingEnemies[NewIndex]);
        int32 AdjacentIndex = NewIndex + 1;
        if (AdjacentIndex >= LivingEnemies.Num()) AdjacentIndex = 0;
        if (LivingEnemies[AdjacentIndex] != LivingEnemies[NewIndex])
        {
            SelectedTargetList.Add(LivingEnemies[AdjacentIndex]);
        }
        break;
    }

    case ETargetingType::All:
        SelectedTargetList = LivingEnemies;
        break;
    }

    if (GameEventComponent)
    {
        GameEventComponent->BroadcastTargetListChanged(SelectedTargetList);
    }
}

void APlayerCharacter::GainEXP(int32 GainedEXP)
{
    if (GetStatsComponent()->GetCurrentHealth() <= 0 || StatsComponent->GetCharacterLevel() >= iMaxLevel) return;

    iCurrentEXP += GainedEXP;
    UE_LOG(LogTemp, Log, TEXT("%s gained %d EXP! (Current EXP: %d / %d)"), *GetName(), GainedEXP, iCurrentEXP, iNextLevelEXP);

    while (iCurrentEXP >= iNextLevelEXP && StatsComponent->GetCharacterLevel() < iMaxLevel)
    {
        LevelUp();
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