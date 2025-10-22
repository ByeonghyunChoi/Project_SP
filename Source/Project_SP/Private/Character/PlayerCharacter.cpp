// Character/PlayerCharacter.cpp

#include "Character/PlayerCharacter.h"
#include "Component/FieldActionComponent.h"
#include "Component/WeaponSystemComponent.h"
#include "Component/PlayerCombatControlComponent.h" 
#include "Component/InventoryComponent.h"
#include "Component/EquipmentSystemComponent.h"
#include "Items/CrystalSkullOparts.h" 
#include "Items/JadeClockOparts.h"
#include "Items/GoldBugOparts.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	FieldActionComp = CreateDefaultSubobject<UFieldActionComponent>(TEXT("FieldActionComponent"));
	WeaponSystemComponent = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystemComponent"));
	CombatControlComponent = CreateDefaultSubobject<UPlayerCombatControlComponent>(TEXT("CombatControlComponent")); // 이름 변경
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	CrystalSkull = CreateDefaultSubobject<UCrystalSkullOparts>(TEXT("CrystalSkullOparts"));
	JadeClock = CreateDefaultSubobject<UJadeClockOparts>(TEXT("JadeClockOparts"));
	GoldBug = CreateDefaultSubobject<UGoldBugOparts>(TEXT("GoldBergOparts"));
	EquipmentSystemComponent = CreateDefaultSubobject<UEquipmentSystemComponent>(TEXT("EquipmentSystemComp"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetFaction(EFaction::Player);

	// 필드 모드로 시작
	OnEnterFieldMode();

	// 오파츠 상태 로그 출력
	//LogOpartsActiveState();

	// EquipmentSystemComponent가 3개의 오파츠 포인터를 참조하도록 초기화
	if (EquipmentSystemComponent)
	{
		// 3개의 미리 장착된 오파츠 포인터를 Manager에 넘겨줍니다.
		EquipmentSystemComponent->InitializeOpartsPointers(
			CrystalSkull,
			JadeClock,
			GoldBug
		);

		// 초기 오파츠 활성화 (예시: 수정 해골로 시작)
		EquipmentSystemComponent->ActivateLastOparts();
	}
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

void APlayerCharacter::LogOpartsActiveState() const
{
	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}
}
