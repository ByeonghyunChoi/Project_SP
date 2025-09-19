#include "Character/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/GameEventComponent.h"
#include "Component/WeaponSystemComponent.h"
#include "Component/FieldModeComponent.h"
#include "Component/ActionComponent.h"
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

void APlayerCharacter::OnTurnBegin()
{
    // 사망 상태가 아니면 행동을 시작할 준비
    if (GetCombatPawnState() != ECombatPawnState::Defeated)
    {
        UE_LOG(LogTemp, Log, TEXT("Player Turn Began."));

        // 1. 자신의 상태를 '행동 선택 중'으로 변경
        SetCombatPawnState(ECombatPawnState::SelectingAction);
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

