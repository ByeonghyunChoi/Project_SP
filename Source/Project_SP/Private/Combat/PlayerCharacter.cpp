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

void APlayerCharacter::OnTurnBegin()
{
}



void APlayerCharacter::HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState)
{
    if (Pawn == this) // 내 Pawn 상태가 변경되었을 때
    {
        // TODO: 상태에 따른 UI 피드백 (예: SelectingTarget 상태 시 타겟 선택 UI 활성화)
    }
}


// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    SetFaction(EFaction::Player);
    if (GameEventComponent)
    {
        GameEventComponent->OnCombatPawnStateChanged.AddDynamic(this, &APlayerCharacter::HandleMyPawnStateChanged);
    }
}