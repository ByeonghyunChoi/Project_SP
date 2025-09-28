#include "Character/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/GameEventComponent.h"
#include "Component/BattleTurnComponent.h"
#include "Component/WeaponSystemComponent.h"
#include "Component/FieldModeComponent.h"
#include "Component/ActionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (GameEventComponent)
    {
        GameEventComponent->OnCombatPawnStateChanged.AddDynamic(this, &APlayerCharacter::HandleMyPawnStateChanged);
    }

    // 1. 자신의 진영을 '플레이어'로 설정
    SetFaction(EFaction::Player);

    // 2. ActionComponent 초기화
    // 플레이어는 무기에서 스킬을 얻으므로, 기본 스킬 목록은 비워둔 채로 초기화합니다.
    if (ActionComponent)
    {
        ActionComponent->InitializeDefaultActions({});
    }

    this->OnEnterFieldMode();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // 각 IA와 핸들러 함수를 연결(바인딩)합니다.
        EnhancedInputComponent->BindAction(IA_SelectBasicAttack, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleSelectBasicAttack);
        EnhancedInputComponent->BindAction(IA_SelectMainSkill, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleSelectMainSkill);
        EnhancedInputComponent->BindAction(IA_Weapon1, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleWeapon1);
        EnhancedInputComponent->BindAction(IA_Weapon2, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleWeapon2);
        EnhancedInputComponent->BindAction(IA_Weapon3, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleWeapon3);
        EnhancedInputComponent->BindAction(IA_ConfirmAction, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleConfirmAction);
        EnhancedInputComponent->BindAction(IA_SelectTargetMouse, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleSelectTargetMouse);
        EnhancedInputComponent->BindAction(IA_CycleTarget, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleCycleTarget);
    }
}


void APlayerCharacter::OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets)
{
    if (GetCombatPawnState() != ECombatPawnState::Defeated)
    {
        UE_LOG(LogTemp, Log, TEXT("Player Turn Began."));

        // BattleManager로부터 받은 타겟 목록을 자신의 변수에 저장합니다.
        AllEnemyTargets = PotentialTargets;

        SetCombatPawnState(ECombatPawnState::SelectingAction);

        SelectedActionID = NAME_None;
    }
}

void APlayerCharacter::RequestSwitchWeapon(EDamageType WeaponType)
{
    // 실제 로직은 WeaponSystemComponent에 완전히 위임
    if (WeaponSystemComponent)
    {
        WeaponSystemComponent->SwitchWeapon(WeaponType);
    }
}

void APlayerCharacter::RequestStartAction(FName ActionID)
{
    // 유효성 검사: 행동 선택이 가능한 상태일 때만 실행
    if (GetCombatPawnState() != ECombatPawnState::SelectingAction && GetCombatPawnState() != ECombatPawnState::SelectingTarget)
    {
        return;
    }

    // 실제 로직은 ActionComponent에 완전히 위임
    if (ActionComponent)
    {
        if (ActionComponent->StartActionByID(this, ActionID, CurrentTargets))
        {
            // 액션 시작에 성공하면 상태를 '행동 수행 중'으로 변경
            SetCombatPawnState(ECombatPawnState::PerformingAction);
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

void APlayerCharacter::SetCurrentTargets(const TArray<ACombatPawn*>& NewTargets)
{
    CurrentTargets = NewTargets;

    // 타겟을 선택했으므로 상태 변경
    SetCombatPawnState(ECombatPawnState::SelectingTarget);
}

void APlayerCharacter::HandleSelectBasicAttack(const FInputActionValue& Value){ SelectAction(FName("Action_MainSkill")); }
void APlayerCharacter::HandleSelectMainSkill(const FInputActionValue& Value) { SelectAction(FName("Action_MainSkill")); }
void APlayerCharacter::HandleWeapon1(const FInputActionValue& Value) { HandleWeaponInput(1); }
void APlayerCharacter::HandleWeapon2(const FInputActionValue& Value) { HandleWeaponInput(2); }
void APlayerCharacter::HandleWeapon3(const FInputActionValue& Value) { HandleWeaponInput(3); }
void APlayerCharacter::HandleConfirmAction(const FInputActionValue& Value) { ConfirmAndExecuteAction(); }
void APlayerCharacter::HandleCycleTarget(const FInputActionValue& Value) { CycleTarget(Value.Get<float>()); }
void APlayerCharacter::HandleSelectTargetMouse(const FInputActionValue& Value) { SelectTargetWithMouse(); }

void APlayerCharacter::SelectAction(FName ActionID)
{
    if (GetCombatPawnState() != ECombatPawnState::SelectingAction || !ActionComponent) return;

    const FActionData* FoundData = ActionComponent->GetActionData(ActionID);
    if (FoundData)
    {
        SelectedActionID = ActionID;
        SetCombatPawnState(ECombatPawnState::SelectingTarget);

        // UI에 신호 보내기
        OnActionSelectedForTargeting.Broadcast(*FoundData);

        // 타겟 선택 시작
        BeginTargetSelection();
    }
}

void APlayerCharacter::ConfirmAndExecuteAction()
{
    if (GetCombatPawnState() != ECombatPawnState::SelectingTarget || SelectedActionID.IsNone()) return;
    if (CurrentTargets.IsEmpty() || !CurrentTargets[0]) return;

    RequestStartAction(SelectedActionID);
    SelectedActionID = NAME_None;
}

void APlayerCharacter::BeginTargetSelection()
{
    if (AllEnemyTargets.Num() > 0)
    {
        CurrentTargetIndex = 0;
        SetCurrentTargets({ AllEnemyTargets[CurrentTargetIndex] });

        // 타겟이 설정되었음을 UI에 알림
        OnTargetChanged.Broadcast(AllEnemyTargets[CurrentTargetIndex]);
    }
}

void APlayerCharacter::CycleTarget(float Direction)
{
    if (GetCombatPawnState() != ECombatPawnState::SelectingTarget || AllEnemyTargets.Num() <= 1 || Direction == 0.0f) return;

    CurrentTargetIndex += FMath::RoundToInt(Direction);

    if (CurrentTargetIndex >= AllEnemyTargets.Num()) CurrentTargetIndex = 0;
    else if (CurrentTargetIndex < 0) CurrentTargetIndex = AllEnemyTargets.Num() - 1;

    SetCurrentTargets({ AllEnemyTargets[CurrentTargetIndex] });

    // 타겟이 변경되었음을 UI에 알림
    OnTargetChanged.Broadcast(AllEnemyTargets[CurrentTargetIndex]);
}

void APlayerCharacter::SelectTargetWithMouse()
{
    if (GetCombatPawnState() != ECombatPawnState::SelectingTarget) return;

    APlayerController* PC = GetController<APlayerController>();
    if (!PC) return;

    FHitResult HitResult;
    if (PC->GetHitResultUnderCursor(ECC_Pawn, false, HitResult))
    {
        ACombatPawn* HitPawn = Cast<ACombatPawn>(HitResult.GetActor());
        if (HitPawn && AllEnemyTargets.Contains(HitPawn))
        {
            // 1. 현재 타겟을 클릭한 폰으로 설정합니다.
            SetCurrentTargets({ HitPawn });

            // 2. 키보드 순환을 위해 인덱스도 업데이트합니다.
            AllEnemyTargets.Find(HitPawn, CurrentTargetIndex);

            // 3. 타겟이 변경되었음을 UI에 알립니다.
            OnTargetChanged.Broadcast(HitPawn);
        }
    }
}

void APlayerCharacter::HandleWeaponInput(int32 WeaponIndex)
{
    EDamageType SelectedWeaponType;
    switch (WeaponIndex)
    {
    case 1: SelectedWeaponType = EDamageType::Fenrir; break;
    case 2: SelectedWeaponType = EDamageType::Surtr; break;
    case 3: SelectedWeaponType = EDamageType::Jormungandr; break;
    default: return;
    }

    // 자신의 BattleTurnComponent를 통해 현재 내 턴인지 확인합니다.
    if (BattleTurnComponent && BattleTurnComponent->IsMyTurn())
    {
        // 내 턴일 경우: 즉시 무기 교체
        RequestSwitchWeapon(SelectedWeaponType);
    }
    else
    {
        // 적 턴일 경우: 스위치 패리 시도
        AttemptSwitchParry(SelectedWeaponType);
    }
}

void APlayerCharacter::AttemptSwitchParry(EDamageType ParryType)
{
    RequestSwitchWeapon(ParryType);
    UE_LOG(LogTemp, Log, TEXT("Attempting Switch Parry with %s"), *UEnum::GetValueAsString(ParryType));

    // TODO: 여기에 실제 패리 로직을 구현합니다.
}