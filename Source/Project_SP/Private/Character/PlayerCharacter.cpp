// Character/PlayerCharacter.cpp

#include "Character/PlayerCharacter.h"
#include "Component/FieldActionComponent.h"
#include "Component/WeaponSystemComponent.h"
#include "Component/PlayerCombatControlComponent.h" 
#include "Component/InventoryComponent.h"
#include "Component/EquipmentSystemComponent.h"
#include "Component/RelicManagerComponent.h"
#include "Items/CrystalSkullOparts.h" 
#include "Items/JadeClockOparts.h"
#include "Items/GoldBugOparts.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Interface/InteractableInterface.h"
#include "InputAction.h"

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
	RelicManagerComponent = CreateDefaultSubobject<URelicManagerComponent>(TEXT("RelicManagerComponent"));
	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);

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

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionVolumeBeginOverlap);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionVolumeEndOverlap);

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

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::OnInteractInput);
		}

		if (FieldAttackAction)
		{
			EnhancedInputComponent->BindAction(FieldAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnFieldAttackInput);
		}
	}
}

void APlayerCharacter::OnInteractInput()
{
	UE_LOG(LogTemp, Warning, TEXT("=== 1. OnInteractInput() 호출됨! (키 눌림) ==="));
	if (OverlappedInteractables.Num() > 0)
	{
		// 0번째 대상(가장 먼저 감지된 대상)을 가져옴
		TScriptInterface<IInteractableInterface> Target = OverlappedInteractables[0];

		// FieldActionComponent가 있는지 확인
		if (FieldActionComp && Target)
		{
			// FieldActionComponent에게 "이 대상과 상호작용해"라고 로직 실행을 '요청'
			FieldActionComp->PerformInteraction(Target.GetInterface());
		}
	}
}

void APlayerCharacter::OnFieldAttackInput()
{
	if (FieldActionComp)
	{
		FieldActionComp->StartAttackSequence();
	}
}

void APlayerCharacter::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("=== 2. Overlap 감지됨!: %s ==="), *OtherActor->GetName());
	if (OtherActor && OtherActor->Implements<UInteractableInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("=== 3. 인터페이스 확인! 배열에 추가: %s ==="), *OtherActor->GetName());
		// 큐(배열)의 맨 뒤에 추가
		OverlappedInteractables.Add(OtherActor);

		UpdateInteractionUI();
	}
}

void APlayerCharacter::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->Implements<UInteractableInterface>())
	{
		// 큐(배열)에서 제거
		OverlappedInteractables.Remove(OtherActor);

		UpdateInteractionUI();
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

void APlayerCharacter::UpdateInteractionUI()
{
	if (OverlappedInteractables.Num() > 0)
	{
		// 1. 큐에 대상이 하나 이상 있음 (0번째 대상 표시)
		if (IInteractableInterface* Interface = OverlappedInteractables[0].GetInterface())
		{
			FText InteractText = Interface->GetInteractText();
			OnInteractableTargetChanged.Broadcast(InteractText); // "이 텍스트를 띄워"
		}
	}
	else
	{
		// 2. 큐가 비어있음
		OnInteractableTargetChanged.Broadcast(FText::GetEmpty()); // "비어있는 텍스트" (UI 숨김 신호)
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
