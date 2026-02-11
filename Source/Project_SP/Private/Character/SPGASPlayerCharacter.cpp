// Character/PlayerCharacter.cpp

#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"
#include "Component/SPInteractionComponent.h"
#include "Character/SPGASPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Map/MapManagerSubSystem.h"


ASPGASPlayerCharacter::ASPGASPlayerCharacter()
{
	InteractionComponent = CreateDefaultSubobject<USPInteractionComponent>(TEXT("InteractComponent"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ASPGASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASPGASPlayerState* SPGAS = GetPlayerState<ASPGASPlayerState>();
	if (SPGAS)
	{
		ASC = SPGAS->GetAbilitySystemComponent();
		AttributeSet = SPGAS->GetAttributeSet();
		ASC->InitAbilityActorInfo(SPGAS, this);
		GiveAbilities();

		UE_LOG(LogTemp, Warning, TEXT("[Server] GAS Initialized & Abilities Given"));
	}

	if (ASC)
	{
		// 1. 기본은 필드 태그
		FGameplayTag ModeTag = FSPGameplayTags::Get().State_Mode_Field;

		// 2. 매니저에게 현재 상태 확인
		UGameInstance* GI = GetGameInstance();
		if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
		{
			// ★ "지금 전투 맵에 있나요?" (bIsBattleActive 확인)
			if (MapManager->IsInBattleMap())
			{
				ModeTag = FSPGameplayTags::Get().State_Mode_Battle;
			}
		}

		// 3. 결정된 태그 부착
		ASC->AddLooseGameplayTag(ModeTag);

		UE_LOG(LogTemp, Log, TEXT("🏷️ Input Mode Initialized: %s"), *ModeTag.ToString());
	}

	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		if (GetAbilitySystemComponent() && AttributeSet)
		{
			// 로드는 무조건 GAS 초기화 이후에!
			// (만약 SaveSystem이 GameInstance에 있다면 여기서 호출)
			USPSaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>();
			if (SaveSystem)
			{
				SaveSystem->LoadPlayerStats(this);
			}
		}
	}

	APlayerController* PlayerController = CastChecked<ASPGASPlayerController>(NewController);
	PlayerController->ConsoleCommand(TEXT("showdebug abilitysystem"));
}

void ASPGASPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// [클라이언트] 초기화 진입
	ASPGASPlayerState* SPGAS = GetPlayerState<ASPGASPlayerState>();
	if (SPGAS)
	{
		ASC = SPGAS->GetAbilitySystemComponent();
		AttributeSet = SPGAS->GetAttributeSet();

		// 클라이언트 쪽 ASC 연결 (이게 있어야 컨트롤러가 ASC를 찾음)
		ASC->InitAbilityActorInfo(SPGAS, this);

		UE_LOG(LogTemp, Warning, TEXT("[Client] GAS Initialized for %s"), *GetName());
	}
}

void ASPGASPlayerCharacter::GiveAbilities()
{
	// 1. 권한 확인 로그
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("GiveAbilities Failed: Not Authority (Client tried to run this?)"));
		return;
	}
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("GiveAbilities Failed: ASC is NULL"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== GiveAbilities Start: %s ==="), *GetName());

	// 2. 필드 입력 GA 확인
	UE_LOG(LogTemp, Log, TEXT("FieldInputAbilities Count: %d"), FieldInputAbilities.Num());
	for (const auto& Pair : FieldInputAbilities)
	{
		if (Pair.Value)
		{
			FGameplayAbilitySpec Spec(Pair.Value);
			ASC->GiveAbility(Spec);

			UE_LOG(LogTemp, Log, TEXT("   -> Given Ability: %s (Tag: %s)"), *Pair.Value->GetName(), *Pair.Key.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("   -> Failed: Value(Class) is None for Tag: %s"), *Pair.Key.ToString());
		}
	}

	// 3. 전투 입력 GA 확인
	UE_LOG(LogTemp, Log, TEXT("BattleInputAbilities Count: %d"), BattleInputAbilities.Num());
	for (const auto& Pair : BattleInputAbilities)
	{
		if (Pair.Value)
		{
			FGameplayAbilitySpec Spec(Pair.Value);
			ASC->GiveAbility(Spec);
			UE_LOG(LogTemp, Log, TEXT("   -> Given Battle Ability: %s"), *Pair.Value->GetName());
		}
	}

	// 4. 필드 패시브 GA 확인
	UE_LOG(LogTemp, Log, TEXT("FieldPassiveAbilities Count: %d"), FieldPassiveAbilities.Num());
	FGameplayTag FieldTag = FSPGameplayTags::Get().Ability_Type_Field; // 이름 수정 확인 필요 (Ability_Type_Auto_Field 인지 확인)
	for (const auto& AbilityClass : FieldPassiveAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass);
			ASC->GiveAbility(Spec);
			UE_LOG(LogTemp, Log, TEXT("   -> Given Field Passive: %s"), *AbilityClass->GetName());
		}
	}

	// 5. 전투 패시브 GA 확인
	UE_LOG(LogTemp, Log, TEXT("BattlePassiveAbilities Count: %d"), BattlePassiveAbilities.Num());
	FGameplayTag BattleTag = FSPGameplayTags::Get().Ability_Type_Battle;
	for (const auto& AbilityClass : BattlePassiveAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass);
			ASC->GiveAbility(Spec);
			UE_LOG(LogTemp, Log, TEXT("   -> Given Battle Passive: %s"), *AbilityClass->GetName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== GiveAbilities End ==="));

	// 초기 상태 활성화
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FieldTag));
}

void ASPGASPlayerCharacter::SetCameraProfile(const FCameraProfile& Profile)
{
	if (!CameraBoom && !FollowCamera) return;

	CameraBoom->TargetArmLength = Profile.TargetArmLength;
	CameraBoom->SocketOffset = Profile.SocketOffset;
	CameraBoom->SetRelativeRotation(Profile.RelativeRotation);
	CameraBoom->bEnableCameraLag = Profile.bEnableLag;

	FollowCamera->SetRelativeLocation(Profile.CameraRelativeLocation);
	FollowCamera->SetRelativeRotation(Profile.CameraRelativeRotation);
}
