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
#include "Components/WidgetComponent.h"


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

	WeaponWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WeaponWidgetComponent"));
	WeaponWidgetComponent->SetupAttachment(GetCapsuleComponent());
	WeaponWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	ActionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ActionWidgetComponent"));
	ActionWidgetComponent->SetupAttachment(GetCapsuleComponent());
	ActionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	BattlePointWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("BattlePointWidgetComponent"));
	BattlePointWidgetComponent->SetupAttachment(GetCapsuleComponent());
	BattlePointWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
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
		// 기본은 필드 태그
		FGameplayTag ModeTag = FSPGameplayTags::Get().State_Mode_Field;

		// 매니저에게 현재 상태 확인
		UGameInstance* GI = GetGameInstance();
		if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
		{
			if (MapManager->IsInBattleMap())
			{
				ModeTag = FSPGameplayTags::Get().State_Mode_Battle;
			}
		}

		// 결정된 태그 부착
		ASC->AddLooseGameplayTag(ModeTag);
		//임시 이벤트 부착 나중에 제거
		ASC->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ASPGASPlayerCharacter::OnGameplayEffectApplied);
		UE_LOG(LogTemp, Log, TEXT("Input Mode Initialized: %s"), *ModeTag.ToString());
	}

	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		if (GetAbilitySystemComponent() && AttributeSet)
		{
			USPSaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>();
			if (SaveSystem)
			{
				SaveSystem->LoadPlayerStats(this);
			}
		}
	}
	SetCameraProfile(FieldCameraSetting);

	ReportReadyToGameMode();

	APlayerController* PlayerController = CastChecked<ASPGASPlayerController>(NewController);
	PlayerController->ConsoleCommand(TEXT("showdebug abilitysystem"));
}

void ASPGASPlayerCharacter::ActivateCombatAbility(FGameplayTag WeaponTag, ESelectedActionType ActionType, AActor* TargetActor)
{
	// 타겟 저장 (GA가 시작되면 이 변수를 읽어갑니다)
	CurrentCombatTarget = TargetActor;

	// 무기 데이터 확인
	if (!WeaponConfigs.Contains(WeaponTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character: 해당 무기 데이터가 없습니다 (%s)"), *WeaponTag.ToString());
		return;
	}

	UWeaponAbilityData* Data = WeaponConfigs[WeaponTag];
	TSubclassOf<UGameplayAbility> AbilityClassToActivate;

	// 행동 타입에 맞는 클래스 선택 (Enum 활용)
	switch (ActionType)
	{
	case ESelectedActionType::NormalAttack:
		AbilityClassToActivate = Data->NormalAttackAbility;
		break;
	case ESelectedActionType::WeaponSkill:
		AbilityClassToActivate = Data->WeaponSkillAbility;
		break;
	case ESelectedActionType::ParrySkill:
		AbilityClassToActivate = Data->ParrySkillAbility;
		break;
	}

	// 어빌리티 실행
	if (AbilityClassToActivate && ASC)
	{
		// 클래스로 실행 (Payload 없이 실행해도 멤버 변수 CurrentCombatTarget을 읽으면 됨)
		if (ASC->TryActivateAbilityByClass(AbilityClassToActivate))
		{
			UE_LOG(LogTemp, Log, TEXT("[Char] 스킬 발동 성공: %s (Target: %s)"),
				*AbilityClassToActivate->GetName(),
				TargetActor ? *TargetActor->GetName() : TEXT("None"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Char] 스킬 발동 실패: %s (Cost/Cool/Tag 등 확인 필요)"), *AbilityClassToActivate->GetName());
		}
	}
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
	// 권한 확인 로그
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

	// 필드 입력 GA 확인
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

	// 전투 입력 GA 확인
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

	// 필드 패시브 GA 확인
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

	// 전투 패시브 GA 확인
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

	GiveWeaponAbilities();

	UE_LOG(LogTemp, Warning, TEXT("=== GiveAbilities End ==="));

	// 초기 상태 활성화
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FieldTag));
}

void ASPGASPlayerCharacter::GiveWeaponAbilities()
{
	UE_LOG(LogTemp, Log, TEXT("Giving Weapon Abilities... Count: %d"), WeaponConfigs.Num());

	for (const auto& Pair : WeaponConfigs)
	{
		UWeaponAbilityData* Data = Pair.Value;
		if (Data)
		{
			if (Data->NormalAttackAbility) ASC->GiveAbility(FGameplayAbilitySpec(Data->NormalAttackAbility));
			if (Data->WeaponSkillAbility)  ASC->GiveAbility(FGameplayAbilitySpec(Data->WeaponSkillAbility));
			if (Data->ParrySkillAbility)   ASC->GiveAbility(FGameplayAbilitySpec(Data->ParrySkillAbility));

			UE_LOG(LogTemp, Log, TEXT("  -> Weapon Registered: %s"), *Pair.Key.ToString());
		}
	}
}

void ASPGASPlayerCharacter::OnGameplayEffectApplied(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	// 쿨타임 이펙트인지 확인 (이름으로 대충 확인)
	if (Spec.Def && Spec.Def->GetName().Contains(TEXT("Cooldown")))
	{
		UE_LOG(LogTemp, Error, TEXT("쿨타임 감지됨! -----------------"));
		UE_LOG(LogTemp, Error, TEXT(" - 이펙트 이름: %s"), *Spec.Def->GetName());
		UE_LOG(LogTemp, Error, TEXT(" - 스택 개수(턴): %d"), Spec.GetStackCount());

		// 누가 붙였나?
		if (const UGameplayAbility* Ability = Cast<UGameplayAbility>(Spec.GetContext().GetAbility()))
		{
			UE_LOG(LogTemp, Error, TEXT(" - 범인(GA): %s"), *Ability->GetName());
		}

		// C++ 함수(ApplyTurnBasedCooldown)에서 붙인 건지, 자동인지 확인
		if (Spec.DynamicGrantedTags.HasTag(FGameplayTag::RequestGameplayTag("Cooldown.Weapon.Fenrir.Skill")))
		{
			UE_LOG(LogTemp, Warning, TEXT(" 이건 우리가 만든 C++ 수동 쿨타임입니다. (정상)"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT(" 이건 시스템이 몰래 붙인 자동 쿨타임입니다! (범인)"));
		}
	}
}

void ASPGASPlayerCharacter::SetCameraProfile(const FCameraProfile& Profile)
{
	if (!CameraBoom || !FollowCamera) return;

	CameraBoom->TargetArmLength = Profile.TargetArmLength;
	CameraBoom->SocketOffset = Profile.SocketOffset;
	CameraBoom->SetRelativeRotation(Profile.RelativeRotation);
	CameraBoom->bEnableCameraLag = Profile.bEnableLag;

	FollowCamera->SetRelativeLocation(Profile.CameraRelativeLocation);
	FollowCamera->SetRelativeRotation(Profile.CameraRelativeRotation);

	UE_LOG(LogTemp, Log, TEXT("카메라 설정 적용됨! 길이: %f"), Profile.TargetArmLength);
}

ETargetingType ASPGASPlayerCharacter::GetTargetingType(FGameplayTag WeaponTag, ESelectedActionType ActionType) const
{
	if (WeaponConfigs.Contains(WeaponTag))
	{
		UWeaponAbilityData* Data = WeaponConfigs[WeaponTag];
		if (!Data) return ETargetingType::Single;

		switch (ActionType)
		{
		case ESelectedActionType::NormalAttack: return Data->NormalAttackTargeting;
		case ESelectedActionType::WeaponSkill:  return Data->WeaponSkillTargeting;
		case ESelectedActionType::ParrySkill:   return Data->ParrySkillTargeting;
		}
	}
	return ETargetingType::Single;
}

TObjectPtr<UWeaponAbilityData> ASPGASPlayerCharacter::GetWeaponData(FGameplayTag WeaponTag) const
{
	if (const TObjectPtr<UWeaponAbilityData>* FoundData = WeaponConfigs.Find(WeaponTag))
	{
		return *FoundData;
	}
	return nullptr;
}

void ASPGASPlayerCharacter::OnBattleStarted()
{
	Super::OnBattleStarted();
	if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(GetController()))
	{
		PC->SetupAndShowBattleUI();
	}
}
