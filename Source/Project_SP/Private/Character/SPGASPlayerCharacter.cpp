// Character/PlayerCharacter.cpp

#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Tag/SPGameplayTags.h"
#include "Component/SPInteractionComponent.h"
#include "Character/SPGASPlayerController.h"
#include "Character/SPGASMonsterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CineCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Map/MapManagerSubSystem.h"
#include "Components/WidgetComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"



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

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatCineCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CombatCineCamera"));
	CombatCineCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CombatCineCamera->bUsePawnControlRotation = false;
	CombatCineCamera->bAutoActivate = false;

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

		ASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_ActionExecuting, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerCharacter::OnActionTagChanged);

		ASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_Battle_TurnActive, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerCharacter::OnActionTagChanged);

		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ASPGASPlayerCharacter::OnHealthChanged);

		ASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetTimePowerAttribute())
			.AddUObject(this, &ASPGASPlayerCharacter::OnTimePowerChanged);

		if (USPGASAttributeSet* SPAS = Cast<USPGASAttributeSet>(AttributeSet))
		{
			SPAS->OnDamageTakenEvent.RemoveAll(this);
			SPAS->OnDamageTakenEvent.AddUObject(this, &ASPGASCharacterBase::BroadcastDamageText);
		}
		
		GiveAbilities();

		int32 TargetLevel = 1;

		if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
		{
			// 세이브된 런 데이터에서 레벨을 가져옴 (세이브가 없으면 기본값 1)
			TargetLevel = FMath::RoundToInt(SaveSys->GetRunData().Stats.Level);
		}

		// 커브 테이블을 읽어와서 뼈대 스탯 세팅
		ApplyLevelStats(TargetLevel, false);

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
		UE_LOG(LogTemp, Log, TEXT("Input Mode Initialized: %s"), *ModeTag.ToString());
	}

	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->RestorePermDataToPlayer(this);
		SaveSys->RestoreRunDataToPlayer(this);
	}

	SetCameraProfile(FieldCameraSetting);
	ReportReadyToGameMode();

	/*APlayerController* PlayerController = CastChecked<ASPGASPlayerController>(NewController);
	PlayerController->ConsoleCommand(TEXT("showdebug abilitysystem"));*/
}


void ASPGASPlayerCharacter::ActivateCombatAbility(FGameplayTag WeaponTag, ESelectedActionType ActionType, AActor* TargetActor)
{
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
		FGameplayEventData Payload;
		Payload.Instigator = this;       // 공격자 
		Payload.Target = TargetActor;    // 타겟

		FGameplayTag TriggerTag = FSPGameplayTags::Get().Event_Battle_ExecuteAction;
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			// 클래스가 일치하는 바로 그 어빌리티를 찾았다면
			if (Spec.Ability && Spec.Ability->GetClass() == AbilityClassToActivate)
			{
				// 이 특정 스펙(Spec)에만 이벤트(Payload)를 전달하여 단독 실행시킵니다.
				ASC->TriggerAbilityFromGameplayEvent(
					Spec.Handle,
					ASC->AbilityActorInfo.Get(),
					TriggerTag,
					&Payload,
					*ASC
				);
				break; // 목표를 찾아 실행했으니 반복문 종료
			}
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
		ASC->InitAbilityActorInfo(SPGAS, this);

		float CurrentLevel = ASC->GetNumericAttribute(USPGASAttributeSet::GetLevelAttribute());
		ApplyLevelStats(FMath::RoundToInt(CurrentLevel), false);

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

void ASPGASPlayerCharacter::SetCameraProfile(const FCameraProfile& Profile)
{
	if (!CameraBoom || !FollowCamera) return;

	CameraBoom->TargetArmLength = Profile.TargetArmLength;
	CameraBoom->SocketOffset = Profile.SocketOffset;
	CameraBoom->SetRelativeRotation(Profile.RelativeRotation);
	CameraBoom->bEnableCameraLag = Profile.bEnableLag;

	FollowCamera->SetRelativeLocation(Profile.CameraRelativeLocation);
	FollowCamera->SetRelativeRotation(Profile.CameraRelativeRotation);

	CombatCineCamera->SetRelativeLocation(Profile.CameraRelativeLocation);
	CombatCineCamera->SetRelativeRotation(Profile.CameraRelativeRotation);
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

void ASPGASPlayerCharacter::AddExperience(float ExpAmount)
{
	if (!ASC || ExpAmount <= 0.0f) return;

	float CurrentLevel = ASC->GetNumericAttribute(USPGASAttributeSet::GetLevelAttribute());
	if (CurrentLevel >= 30.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("이미 만렙(Lv.30)이므로 경험치를 획득하지 않습니다."));
		return;
	}

	float CurrentExp = ASC->GetNumericAttribute(USPGASAttributeSet::GetExperienceAttribute());
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), CurrentExp + ExpAmount);

	UE_LOG(LogTemp, Log, TEXT("경험치 획득: +%.0f (현재 %.0f)"), ExpAmount, CurrentExp + ExpAmount);

	CheckLevelUp();
}

void ASPGASPlayerCharacter::OnBattleStarted()
{
	Super::OnBattleStarted();

	// 전투 기본 세팅 초기화
	if(ASC)
	{
		float StartingBP = 2.0f;
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute(), StartingBP);

		// 모든 스킬의 쿨타임을 0으로 만듭니다.
		FGameplayTag ContainerTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Weapon"));
		FGameplayEffectQuery Query;
		Query.EffectTagQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(FGameplayTagContainer(ContainerTag));
		ASC->RemoveActiveEffects(Query);

		UE_LOG(LogTemp, Warning, TEXT("[%s] 전투 시작: BP 리필 및 쿨타임 초기화 완료!"), *GetName());
	}

	if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(GetController()))
	{
		PC->SetupAndShowBattleUI();
	}
}

void ASPGASPlayerCharacter::SwitchCameraMode(bool bIsBattle)
{
	if (bIsBattle)
	{
		// 일반 카메라 끄고, 전투 카메라 켜기
		FollowCamera->Deactivate();
		CombatCineCamera->Activate();

		SetCameraProfile(CombatCameraSetting);

		UE_LOG(LogTemp, Log, TEXT("카메라 전환: 전투용 CineCamera 활성화"));
	}
	else
	{
		// 전투 카메라 끄고, 일반 카메라 켜기
		CombatCineCamera->Deactivate();
		FollowCamera->Activate();

		SetCameraProfile(FieldCameraSetting);

		UE_LOG(LogTemp, Log, TEXT("카메라 전환: 필드용 FollowCamera 활성화"));
	}
}

void ASPGASPlayerCharacter::OnActionTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (!ASC) return;

	// 지금 내 턴인가?
	bool bIsMyTurn = ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);

	// 지금 스킬을 실행 중인가?
	bool bIsExecuting = ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_ActionExecuting);

	// 내 턴이면서 행동 중이 아닐 때
	bool bShouldShowUI = (bIsMyTurn && !bIsExecuting);

	// 행동 중이 아닐 때
	bool bShouldShowUIWeapon = !bIsExecuting;

	// 위젯 켜고 끄기 적용
	if (WeaponWidgetComponent)
	{
		WeaponWidgetComponent->SetVisibility(bShouldShowUIWeapon);
	}
	if (ActionWidgetComponent)
	{
		ActionWidgetComponent->SetVisibility(bShouldShowUI);
	}
	if (BattlePointWidgetComponent)
	{
		BattlePointWidgetComponent->SetVisibility(bShouldShowUI);
	}
}


void ASPGASPlayerCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 방금 전까지 체력이 0보다 컸는데, 지금 0 이하가 되었다면? (사망 순간)
	if (Data.NewValue <= 0.0f && Data.OldValue > 0.0f)
	{
		if (ASC)
		{
			float CurrentTimePower = ASC->GetNumericAttribute(USPGASAttributeSet::GetTimePowerAttribute());

			// 부활 기믹 조건 충족
			if (CurrentTimePower >= 20.0f)
			{
				// 1) 시간의 힘 20 삭감
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), CurrentTimePower - 20.0f);

				// 2) 최대 체력의 40%로 부활
				float MaxHealth = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
				float ReviveHealth = MaxHealth * 0.4f;
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), ReviveHealth);

				UE_LOG(LogTemp, Warning, TEXT("[플레이어] 시간의 힘 20 소모하여 부활! (체력: %.0f)"), ReviveHealth);
			}
			else
			{
				// 진짜 사망 (시간의 힘 부족)
				UE_LOG(LogTemp, Error, TEXT("[플레이어] 시간의 힘이 부족하여 사망했습니다."));

				// TODO: 애니메이션 재생, 게임 오버 UI 호출 등
				// Die(); 
			}
		}
	}
}

void ASPGASPlayerCharacter::OnTimePowerChanged(const FOnAttributeChangeData& Data)
{
	// 방금 전까지 0보다 컸는데, 지금 0 이하가 되었다면?
	if (Data.NewValue <= 0.0f && Data.OldValue > 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("[플레이어] 시간의 힘이 모두 고갈되었습니다! 로비로 귀환합니다."));
		UGameInstance* GI = GetGameInstance();
		UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr;
		MapManager->GoToLobby();
	}
}

void ASPGASPlayerCharacter::CheckLevelUp()
{
	if (!ASC || !PlayerStatCurve) return; // 커브가 없으면 레벨업 불가

	float CurrentExp = ASC->GetNumericAttribute(USPGASAttributeSet::GetExperienceAttribute());
	float MaxExp = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxExperienceAttribute());
	float CurrentLevel = ASC->GetNumericAttribute(USPGASAttributeSet::GetLevelAttribute());

	bool bDidLevelUp = false;

	// 경험치가 꽉 찼다면? (한 번에 2업 이상 하는 경우를 대비해 while문 사용)
	while (MaxExp > 0.0f && CurrentExp >= MaxExp && CurrentLevel < 30.0f)
	{
		CurrentExp -= MaxExp; // 경험치 초과분 이월
		CurrentLevel += 1.0f; // 레벨 1 증가

		bDidLevelUp = true;

		// 갱신된 레벨의 스탯과 MaxExp를 커브에서 즉시 읽어오기!
		ApplyLevelStats(FMath::RoundToInt(CurrentLevel), true);

		// 🌟 [추가] 방금 레벨업해서 30레벨이 되었다면? 초과분은 증발시키고 루프 강제 종료!
		if (CurrentLevel >= 30.0f)
		{
			CurrentExp = 0.0f;
			UE_LOG(LogTemp, Warning, TEXT("만렙(Lv.30) 달성! 더 이상 경험치가 오르지 않습니다."));
			break;
		}

		// 갱신된 다음 레벨의 MaxExp를 다시 가져와서 while문 조건 재검사
		MaxExp = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxExperienceAttribute());
	}

	// 초과분을 깎은(혹은 만렙이라 0이 된) 최종 경험치 저장
	if (bDidLevelUp)
	{
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), CurrentExp);
		OnLevelUpEffect();
	}
}

void ASPGASPlayerCharacter::ApplyLevelStats(int32 TargetLevel, bool bIsLevelUp)
{
	if (!ASC) return;

	// 커브 테이블에서 스탯 뽑아오기 (무조건 실행)
	if (PlayerStatCurve)
	{
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute(), TargetLevel);

		FString ContextString = TEXT("PlayerStatGrowth");

		float RequiredExp = PlayerStatCurve->FindCurve(FName("Exp"), ContextString)->Eval(TargetLevel);
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxExperienceAttribute(), RequiredExp);
		float MaxHP = PlayerStatCurve->FindCurve(FName("HP"), ContextString)->Eval(TargetLevel);
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute(), MaxHP);
		float Attack = PlayerStatCurve->FindCurve(FName("Attack"), ContextString)->Eval(TargetLevel);
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute(), Attack);
		float Defense = PlayerStatCurve->FindCurve(FName("Defense"), ContextString)->Eval(TargetLevel);
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute(), Defense);
		float CriticalRate = PlayerStatCurve->FindCurve(FName("CriticalRate"), ContextString)->Eval(TargetLevel);
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCriticalRateAttribute(), CriticalRate);
		float Speed = PlayerStatCurve->FindCurve(FName("Speed"), ContextString)->Eval(TargetLevel);
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute(), Speed);
		
		// 레벨업 시 체력 100% 회복
		if (bIsLevelUp)
		{
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), MaxHP);
		}
	}

	// 데이터 테이블에서 보상 뽑아오기
	if (PlayerRewardTable && bIsLevelUp)
	{
		// 지금은 비워 둠
	}
}
