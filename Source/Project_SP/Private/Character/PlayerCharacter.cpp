// Character/PlayerCharacter.cpp

#include "Character/PlayerCharacter.h"
#include "Component/FieldActionComponent.h"
#include "Component/WeaponSystemComponent.h"
#include "Component/PlayerCombatControlComponent.h" 
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	FieldActionComp = CreateDefaultSubobject<UFieldActionComponent>(TEXT("FieldActionComponent"));
	WeaponSystemComponent = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystemComponent"));
	CombatControlComponent = CreateDefaultSubobject<UPlayerCombatControlComponent>(TEXT("CombatControlComponent")); // 이름 변경

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetFaction(EFaction::Player);

	// 필드 모드로 시작
	OnEnterFieldMode();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 입력 설정의 책임을 CombatControlComponent에게 위임합니다.
		if (CombatControlComponent)
		{
			CombatControlComponent->SetupPlayerInput(EnhancedInputComponent);
		}
	}
}

void APlayerCharacter::OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets)
{
	if (GetCombatPawnState() == ECombatPawnState::Defeated) return;

	UE_LOG(LogTemp, Log, TEXT("Player Turn Began."));

	// 턴 시작의 책임을 CombatControlComponent에게 위임합니다.
	if (CombatControlComponent)
	{
		CombatControlComponent->OnTurnBegin(PotentialTargets);
	}
}