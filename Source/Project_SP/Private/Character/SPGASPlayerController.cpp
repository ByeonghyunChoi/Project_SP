// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SPGASPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Character/SPGASPlayerState.h"
#include "Character/SPGASPlayerCharacter.h"  
#include "Character/SPGASMonsterCharacter.h" 
#include "Tag/SPGameplayTags.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "GameplayEffect.h"
#include "Data/Asset/WeaponAbilityData.h"
#include "Map/MapManagerSubSystem.h"
#include "Framework/Application/SlateApplication.h"

ASPGASPlayerController::ASPGASPlayerController()
{
	bShowMouseCursor = true;
	CurrentSelectedAction = ESelectedActionType::None;
}

void ASPGASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 설정 (필드/전투 모두 사용)
	FInputModeGameAndUI InputModeData;
	InputModeData.SetHideCursorDuringCapture(false);
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);

	bool bIsBattleMap = false;
	UGameInstance* GI = GetGameInstance();
	if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
	{
		bIsBattleMap = MapManager->IsInBattleMap();
	}

	if (FieldHUDClass && !FieldHUDWidget)
	{
		FieldHUDWidget = CreateWidget<UUserWidget>(this, FieldHUDClass);
		if (FieldHUDWidget)
		{
			FieldHUDWidget->AddToViewport();

			// 전투 맵이라면 처음부터 필드 UI를 숨긴 채로 만들고, 필드일 때만 켭니다!
			FieldHUDWidget->SetVisibility(bIsBattleMap ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible);
		}
	}

	if (BattleHUDClass && !BattleHUDWidget)
	{
		BattleHUDWidget = CreateWidget<UUserWidget>(this, BattleHUDClass);
		if (BattleHUDWidget)
		{
			BattleHUDWidget->AddToViewport();
			BattleHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ASPGASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 1. [필드] 이동 (MoveAction - WASD)
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASPGASPlayerController::OnMove);
		}

		// 2. [전투] 타겟 변경 
		if (BattleNavigateAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("IA_BattleNavigate가 정상적으로 바인딩 되었습니다!")); 
			EIC->BindAction(BattleNavigateAction, ETriggerEvent::Triggered, this, &ASPGASPlayerController::OnBattleNavigate);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("IA_BattleNavigate가 비어있습니다! BP_PlayerController를 확인하세요.")); 
		}

		// 3. [필드] 일반 액션 바인딩
		for (const FSPInputConfig& Config : FieldInputConfigs)
		{
			if (Config.InputAction && Config.InputTag.IsValid())
			{
				EIC->BindAction(Config.InputAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnFieldInputPressed, Config.InputTag);
			}
		}

		// 4. [전투] 전투 액션 바인딩 
		for (const FSPInputConfig& Config : BattleInputConfigs)
		{
			if (Config.InputAction && Config.InputTag.IsValid())
			{
				EIC->BindAction(Config.InputAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnBattleInputPressed, Config.InputTag);
			}
		}

		// 전투 마우스 클릭 로직 바인딩
		if (BattleClickAction)
		{
			EIC->BindAction(BattleClickAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnBattleClick);
		}
	}
}

void ASPGASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitAbilitySystem(InPawn);
}

void ASPGASPlayerController::InitAbilitySystem(APawn* InPawn)
{
	if (!InPawn) return;

	// 1. ASC 가져오기
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InPawn))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
	}
	else if (ASPGASPlayerState* PS = GetPlayerState<ASPGASPlayerState>())
	{
		CachedASC = PS->GetAbilitySystemComponent();
	}

	if (CachedASC)
	{
		// 전투 태그
		FGameplayTag BattleTag = FSPGameplayTags::Get().State_Mode_Battle;

		// 이벤트 등록 (앞으로의 변화를 감지하기 위해 등록)
		CachedASC->RegisterGameplayTagEvent(BattleTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerController::OnBattleTagChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetBattlePointAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnBattlePointChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetMaxBattlePointAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnMaxBattlePointChanged);

		// 현재 상태를 확인해서, 강제로 콜백 함수 호출!
		bool bIsBattle = CachedASC->HasMatchingGameplayTag(BattleTag);

		//
		OnBattleTagChanged(BattleTag, bIsBattle ? 1 : 0);

		UE_LOG(LogTemp, Warning, TEXT("InitAbilitySystem: 초기화 완료 (강제 호출 수행함)"));
	}
}

void ASPGASPlayerController::OnMove(const FInputActionValue& Value)
{
	// 필드 전용 이동 로직
	if (CachedASC && CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_BlockMove)) return;

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector2D Vector = Value.Get<FVector2D>();
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		ControlledPawn->AddMovementInput(ForwardDirection, Vector.Y);
		ControlledPawn->AddMovementInput(RightDirection, Vector.X);
	}
}

void ASPGASPlayerController::OnBattleNavigate(const FInputActionValue& Value)
{
	// 전투 전용 타겟 변경 로직 (타겟팅 모드일 때만 작동)
	if (!bIsSelectingTarget || AvailableTargets.Num() == 0) return;

	if (CurrentTargetingType == ETargetingType::All)
	{
		return;
	}

	float Direction = Value.Get<float>();

	if (FMath::IsNearlyZero(Direction)) return;
	UE_LOG(LogTemp, Warning, TEXT("키 입력 감지됨! 값: %f"), Direction);

	// 1. 기존 타겟 하이라이트 끄기
	HighlightCurrentTarget(false);

	// 2. 인덱스 계산 (좌우 순환)
	if (Direction > 0) // 오른쪽 (D)
	{
		CurrentTargetIndex = (CurrentTargetIndex + 1) % AvailableTargets.Num();
	}
	else // 왼쪽 (A)
	{
		// 음수 모듈러 연산 보정
		CurrentTargetIndex = (CurrentTargetIndex - 1 + AvailableTargets.Num()) % AvailableTargets.Num();
	}

	// 3. 새 타겟 하이라이트 켜기
	HighlightCurrentTarget(true);

	UE_LOG(LogTemp, Log, TEXT("타겟 변경: [%d] %s"), CurrentTargetIndex, *AvailableTargets[CurrentTargetIndex]->GetName());
}

void ASPGASPlayerController::OnFieldInputPressed(FGameplayTag InputTag)
{
	if (!CachedASC) return;
	// 필드는 즉시 실행
	CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}

void ASPGASPlayerController::OnBattleInputPressed(FGameplayTag InputTag)
{
	// 1. 턴 체크
	if (!IsMyTurn())
	{
		UE_LOG(LogTemp, Warning, TEXT("아직 내 턴이 아닙니다."));
		return;
	}

	const FSPGameplayTags& GameplayTags = FSPGameplayTags::Get();

	// 2. 무기 교체 입력 (Weapon.*)
	if (InputTag.MatchesTag(FGameplayTag::RequestGameplayTag("Weapon")))
	{
		if (CurrentWeaponTag == InputTag)
		{
			UE_LOG(LogTemp, Log, TEXT("이미 장착 중인 무기입니다. 입력을 무시합니다."));
			return;
		}
		// 타겟팅 중에 무기를 바꾸면 타겟팅 취소
		if (bIsSelectingTarget) CancelTargetSelection();

		ProcessWeaponSwitch(InputTag);
		return;
	}

	if (InputTag.MatchesTag(FGameplayTag::RequestGameplayTag("Battle.Action.TimeInterference")))
	{
		if (bIsSelectingTarget) CancelTargetSelection(); // 타겟팅 중이었다면 취소

		// 이미 시간 간섭 상태인지 확인 (중복 발동 방지)
		// [수정] 버프 태그는 "State.Buff..." 로 확인!
		if (CachedASC && CachedASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.TimeInterference")))
		{
			UE_LOG(LogTemp, Warning, TEXT("[시스템] 이미 시간 간섭이 발동 중입니다!"));
			return;
		}

		// 쿨타임 검사 (선생님이 추가하신 태그 아주 좋습니다!)
		if (CachedASC && CachedASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Cooldown.Skill.TimeInterference")))
		{
			UE_LOG(LogTemp, Warning, TEXT("[시스템] 시간 간섭 스킬이 쿨타임 중입니다! (남은 턴 대기)"));
			return;
		}

		int32 Cost = GetSkillCost(InputTag);
		if (GetCurrentTimePower() < Cost)
		{
			UE_LOG(LogTemp, Warning, TEXT("[시스템] 시간의 힘이 부족하여 발동할 수 없습니다!"));
			return; // 실행 취소!
		}

		// 시간 간섭 GA 실행 시도
		if (CachedASC)
		{
			bool bSuccess = CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
			if (bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 발동!"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 발동 실패! (시간의 힘 부족 등)"));
			}
		}
		return;
	}

	// 3. 행동 선택 입력 (Attack, Skill, Parry)
	ESelectedActionType InputType = ESelectedActionType::None;

	if (InputTag.MatchesTag(GameplayTags.Battle_Action_Attack)) InputType = ESelectedActionType::NormalAttack;
	else if (InputTag.MatchesTag(GameplayTags.Battle_Action_Skill)) InputType = ESelectedActionType::WeaponSkill;
	else if (InputTag.MatchesTag(GameplayTags.Battle_Action_Parry)) InputType = ESelectedActionType::ParrySkill;

	if (InputType != ESelectedActionType::None)
	{
		// 무기 미착용 체크
		if (!CurrentWeaponTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("무기를 먼저 선택하세요! (키: 1, 2, 3)"));
			return;
		}

		if (InputType == ESelectedActionType::WeaponSkill)
		{
			// 1. 쿨타임 검사
			if (GetSkillCooldownTurns(GameplayTags.Battle_Action_Skill) > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[시스템] 무기 스킬 쿨타임 중입니다!"));
				return; // 타겟팅 진입 차단!
			}

			// 2. BP 및 프리패스(시간 간섭) 검사
			int32 Cost = GetSkillCost(GameplayTags.Battle_Action_Skill);
			bool bIsTimeInterference = CachedASC && CachedASC->HasMatchingGameplayTag(GameplayTags.State_TimeInterference);

			// 시간 간섭 버프가 없는데, BP마저 부족하다면?
			if (!bIsTimeInterference && GetCurrentBP() < Cost)
			{
				UE_LOG(LogTemp, Warning, TEXT("[시스템] BP가 부족합니다!"));
				return; // 타겟팅 진입 차단!
			}
		}

		if (CachedASC && CachedASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.TimeInterference")))
		{
			// 선택한 행동이 '무기 스킬'이 아니라면? (일반 공격이나 패링이라면)
			if (InputType != ESelectedActionType::WeaponSkill)
			{
				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭 발동 중!] 무기 스킬과 무기 교체만 사용할 수 있습니다."));
				// 행동을 무시하고 함수 종료 (타겟팅으로 안 넘어감)
				return;
			}
		}

		if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
		{
			CurrentTargetingType = PlayerChar->GetTargetingType(CurrentWeaponTag, InputType);
		}

		// [핵심 로직]
		if (CurrentSelectedAction == InputType)
		{
			// A. 이미 선택된 행동을 다시 누름 -> 확정!
			if (bIsSelectingTarget)
			{
				ConfirmTargetAndExecute();
			}
			else
			{
				// 예외 처리: 선택은 됐는데 타겟팅이 꺼져있다면 다시 시작
				StartTargetSelection();
			}
		}
		else
		{
			// B. 새로운 행동을 누름 -> 선택 및 타겟팅 시작!
			if (bIsSelectingTarget) HighlightCurrentTarget(false); // 이전 타겟팅 끄기

			CurrentSelectedAction = InputType;
			UE_LOG(LogTemp, Log, TEXT("행동 선택됨: %d -> 타겟을 선택하세요 (A/D)"), (int32)InputType);

			StartTargetSelection();
		}
	}
}

int32 ASPGASPlayerController::GetSkillCooldownTurns(FGameplayTag SkillTag) const
{
	if (!CachedASC) return 0;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	FGameplayTag CooldownTagToSearch;

	// 1. 어떤 스킬의 쿨타임을 물어보는 건지 파악해서 '검색할 쿨타임 태그'를 결정합니다.
	if (SkillTag.MatchesTag(SPTags.Battle_Action_TimeInterference))
	{
		CooldownTagToSearch = SPTags.Cooldown_Skill_TimeInterference;
	}
	else if (SkillTag.MatchesTag(SPTags.Battle_Action_Skill))
	{
		// 무기 스킬이라면, '현재 들고 있는 무기'가 무엇인지에 따라 쿨타임 태그가 다릅니다!
		if (CurrentWeaponTag.MatchesTag(SPTags.Weapon_Fenrir))
			CooldownTagToSearch = SPTags.Cooldown_Weapon_Fenrir_Skill;
		else if (CurrentWeaponTag.MatchesTag(SPTags.Weapon_Surtr))
			CooldownTagToSearch = SPTags.Cooldown_Weapon_Surtr_Skill;
		else if (CurrentWeaponTag.MatchesTag(SPTags.Weapon_Jormungandr))
			CooldownTagToSearch = SPTags.Cooldown_Weapon_Jormungandr_Skill;
	}

	// 찾는 태그가 없으면 쿨타임 아님(0)
	if (!CooldownTagToSearch.IsValid()) return 0;

	// 2. ASC(내 몸)에서 해당 쿨타임 태그를 부여하고 있는 GE(바구니)를 찾습니다.
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTagToSearch));
	TArray<FActiveGameplayEffectHandle> ActiveEffects = CachedASC->GetActiveEffects(Query);

	// 3. 해당 GE가 존재한다면, 그 GE의 '현재 스택 수(남은 턴 수)'를 반환합니다!
	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		int32 CurrentStacks = CachedASC->GetCurrentStackCount(Handle);
		if (CurrentStacks > 0)
		{
			return CurrentStacks;
		}
	}

	// 없으면 쿨타임이 돌고 있지 않은 상태 (0)
	return 0;
}

int32 ASPGASPlayerController::GetCurrentBP() const
{
	if (!CachedASC) return 0;

	float CurrentBP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetBattlePointAttribute());
	return FMath::FloorToInt(CurrentBP);
}

int32 ASPGASPlayerController::GetMaxBP() const
{
	if (!CachedASC) return 0;

	float MaxBP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxBattlePointAttribute());
	return FMath::FloorToInt(MaxBP);
}

int32 ASPGASPlayerController::GetCurrentTimePower() const
{
	if (!CachedASC) return 0;

	float CurrentTimePower = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetTimePowerAttribute());
	return FMath::FloorToInt(CurrentTimePower);
}

int32 ASPGASPlayerController::GetSkillCost(FGameplayTag ActionTag) const
{
	if (!CachedASC) return 0;

	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	// 1. 시간 간섭은 무기와 상관없는 공용 스킬이므로 기존처럼 '태그'로 찾습니다.
	if (ActionTag.MatchesTag(SPTags.Battle_Action_TimeInterference))
	{
		for (const FGameplayAbilitySpec& Spec : CachedASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetAssetTags().HasTagExact(ActionTag))
			{
				if (const UGameplayEffect* CostGE = Spec.Ability->GetCostGameplayEffect())
				{
					for (const FGameplayModifierInfo& ModInfo : CostGE->Modifiers)
					{
						if (ModInfo.Attribute == USPGASAttributeSet::GetTimePowerAttribute())
						{
							float CostValue = 0.0f;
							if (ModInfo.ModifierMagnitude.GetStaticMagnitudeIfPossible(1.0f, CostValue))
								return FMath::FloorToInt(FMath::Abs(CostValue));
						}
					}
				}
			}
		}
		return 0;
	}

	// 2. 무기 스킬인 경우 -> 태그가 아니라 "클래스"로 찾습니다!
	if (ActionTag.MatchesTag(SPTags.Battle_Action_Skill))
	{
		ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn());
		if (!PlayerChar) return 0;

		// 캐릭터 클래스에서 현재 장착 중인 무기의 DataAsset을 가져오는 함수를 호출
		UWeaponAbilityData* WeaponData = PlayerChar->GetWeaponData(CurrentWeaponTag);

		if (WeaponData && WeaponData->WeaponSkillAbility)
		{
			// DataAsset에 등록된 바로 그 "클래스(TSubclassOf)"를 타겟으로 지정합니다.
			TSubclassOf<UGameplayAbility> TargetAbilityClass = WeaponData->WeaponSkillAbility;

			// ASC를 뒤져서 클래스가 일치하는 녀석을 찾습니다.
			for (const FGameplayAbilitySpec& Spec : CachedASC->GetActivatableAbilities())
			{
				// 태그 비교가 아니라 클래스 비교!
				if (Spec.Ability && Spec.Ability->GetClass() == TargetAbilityClass)
				{
					if (const UGameplayEffect* CostGE = Spec.Ability->GetCostGameplayEffect())
					{
						for (const FGameplayModifierInfo& ModInfo : CostGE->Modifiers)
						{
							if (ModInfo.Attribute == USPGASAttributeSet::GetBattlePointAttribute())
							{
								float CostValue = 0.0f;
								if (ModInfo.ModifierMagnitude.GetStaticMagnitudeIfPossible(1.0f, CostValue))
									return FMath::FloorToInt(FMath::Abs(CostValue));
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

float ASPGASPlayerController::GetHealthPercent() const
{
	if (!CachedASC) return 0.0f;

	float CurrentHP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute());
	float MaxHP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
	if (MaxHP > 0.0f)
	{
		return FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f);
	}
	return 0.0f;
}


float ASPGASPlayerController::GetTimePowerPercent() const
{
	if (!CachedASC) return 0.0f;

	float CurrentTP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetTimePowerAttribute());
	float MaxTP = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxTimePowerAttribute());

	if (MaxTP > 0.0f)
	{
		return FMath::Clamp(CurrentTP / MaxTP, 0.0f, 1.0f);
	}
	return 0.0f;
}

void ASPGASPlayerController::SetupAndShowBattleUI()
{
	// 1. ASC 및 캐릭터 안전하게 쥐기
	ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn());
	if (PlayerChar)
	{
		CachedASC = PlayerChar->GetAbilitySystemComponent();

		// 화면 UI가 켜질 때 위젯 컴포넌트도 여기서 한 번에 켭니다!
		if (PlayerChar->GetWeaponWidgetComponent()) PlayerChar->GetWeaponWidgetComponent()->SetVisibility(true);
		if (PlayerChar->GetActionWidgetComponent()) PlayerChar->GetActionWidgetComponent()->SetVisibility(true);
		if (PlayerChar->GetBattlePointWidgetComponent()) PlayerChar->GetBattlePointWidgetComponent()->SetVisibility(true);
	}

	// 2. 위젯 생성 및 스위치!
	if (!BattleHUDWidget && BattleHUDClass)
	{
		BattleHUDWidget = CreateWidget<UUserWidget>(this, BattleHUDClass);
		if (BattleHUDWidget) BattleHUDWidget->AddToViewport();
	}

	if (FieldHUDWidget) FieldHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	if (BattleHUDWidget) BattleHUDWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UE_LOG(LogTemp, Warning, TEXT("[UI 통합 제어] 전투 화면 UI 및 위젯 컴포넌트 활성화 완료!"));
}

void ASPGASPlayerController::HideBattleUIAndShowFieldUI()
{
	if (BattleHUDWidget) BattleHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	if (FieldHUDWidget) FieldHUDWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 필드로 돌아갈 때 전투용 위젯 컴포넌트도 깔끔하게 숨깁니다.
	if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
	{
		if (PlayerChar->GetWeaponWidgetComponent()) PlayerChar->GetWeaponWidgetComponent()->SetVisibility(false);
		if (PlayerChar->GetActionWidgetComponent()) PlayerChar->GetActionWidgetComponent()->SetVisibility(false);
		if (PlayerChar->GetBattlePointWidgetComponent()) PlayerChar->GetBattlePointWidgetComponent()->SetVisibility(false);
	}

	// 전투가 끝났으니 타겟팅 하이라이트도 끄고 초기화!
	CancelTargetSelection();

	UE_LOG(LogTemp, Warning, TEXT("[UI 통합 제어] 필드 UI 전환 및 전투 위젯 컴포넌트 비활성화 완료!"));
}

void ASPGASPlayerController::RefreshBattlePointUI()
{
	if (!CachedASC) return;

	int32 CurrentBP = FMath::FloorToInt(CachedASC->GetNumericAttribute(USPGASAttributeSet::GetBattlePointAttribute()));
	int32 MaxBP = FMath::FloorToInt(CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxBattlePointAttribute()));
	OnBattlePointUIUpdated.Broadcast(CurrentBP, MaxBP);
}

void ASPGASPlayerController::UpdateTurnTimelineUI(const TArray<AActor*>& PredictedTurnOrder)
{
	OnTurnOrderUIUpdated.Broadcast(PredictedTurnOrder);
}

void ASPGASPlayerController::StartTargetSelection()
{
	// 1. 적 목록 찾기
	AvailableTargets.Empty();
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), AllActors); // 태그: Enemy

	for (AActor* Actor : AllActors)
	{
		// TODO: 나중에 죽은 적 제외 로직 추가 (IsAlive 등)
		AvailableTargets.Add(Actor);
	}

	if (AvailableTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("공격 가능한 적이 없습니다!"));
		CancelTargetSelection();
		return;
	}

	// 2. 초기화
	bIsSelectingTarget = true;
	CurrentTargetIndex = 0; 

	// 3. 하이라이트 ON
	HighlightCurrentTarget(true);
}

void ASPGASPlayerController::ConfirmTargetAndExecute()
{
	if (AvailableTargets.IsValidIndex(CurrentTargetIndex))
	{
		AActor* SelectedTarget = AvailableTargets[CurrentTargetIndex];

		// 하이라이트 OFF 및 상태 리셋
		HighlightCurrentTarget(false);
		bIsSelectingTarget = false;

		// 실행 명령
		ExecuteBattleAbility(CurrentSelectedAction, SelectedTarget);

		// 행동 초기화 (다음 턴을 위해)
		CurrentSelectedAction = ESelectedActionType::None;
	}
}

void ASPGASPlayerController::CancelTargetSelection()
{
	HighlightCurrentTarget(false);
	bIsSelectingTarget = false;
	CurrentSelectedAction = ESelectedActionType::None;
	AvailableTargets.Empty();
	UE_LOG(LogTemp, Log, TEXT("타겟 선택 취소됨"));
}

void ASPGASPlayerController::HighlightCurrentTarget(bool bHighlight)
{
	if (AvailableTargets.Num() == 0) return;

	// 1. 🌟 [전체 공격(All)] 이라면 -> 모두를 평등하게 '주 타겟(100% 크기)'으로 켭니다!
	if (CurrentTargetingType == ETargetingType::All)
	{
		for (AActor* Target : AvailableTargets)
		{
			if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(Target))
			{
				// 모두가 큼지막한 주 타겟 마커를 갖게 됩니다.
				Monster->SetSelectedWidget(bHighlight, true);
			}
		}
	}
	// 2. 💥 [광역 공격(Area)] 이라면 -> A/D로 선택한 놈만 주 타겟(크게), 나머진 보조 타겟(작게)!
	else if (CurrentTargetingType == ETargetingType::Area)
	{
		for (int32 i = 0; i < AvailableTargets.Num(); ++i)
		{
			if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(AvailableTargets[i]))
			{
				bool bIsPrimary = (i == CurrentTargetIndex);
				Monster->SetSelectedWidget(bHighlight, bIsPrimary);
			}
		}
	}
	// 3. 🎯 [단일(Single) / 랜덤(Random)] 이라면 -> 현재 인덱스 한 명만!
	else
	{
		if (AvailableTargets.IsValidIndex(CurrentTargetIndex))
		{
			if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(AvailableTargets[CurrentTargetIndex]))
			{
				Monster->SetSelectedWidget(bHighlight, true);
			}
		}
	}
}

void ASPGASPlayerController::ExecuteBattleAbility(ESelectedActionType ActionType, AActor* TargetActor)
{
	if (ASPGASPlayerCharacter* PlayerCharacter = Cast<ASPGASPlayerCharacter>(GetPawn()))
	{
		UE_LOG(LogTemp, Warning, TEXT("EXECUTE! 무기: %s, 타겟: %s"),
			*CurrentWeaponTag.ToString(), *TargetActor->GetName());

		// 캐릭터에게 실행 요청 (타겟 정보 전달)
		PlayerCharacter->ActivateCombatAbility(CurrentWeaponTag, ActionType, TargetActor);
	}
}

void ASPGASPlayerController::OnBattleClick(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("=== 1. 마우스 클릭 버튼 눌림! ==="));
	// 타겟팅 모드 검사
	if (!bIsSelectingTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("X. 타겟팅 모드가 아니라서 취소됨"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("=== 2. 타겟팅 모드 통과! ==="));

	// 마우스 커서 아래에 있는 물체(액터)를 쏩니다
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_GameTraceChannel2, false, HitResult))
	{
		AActor* ClickedActor = HitResult.GetActor();
		UE_LOG(LogTemp, Warning, TEXT("=== 4. 마우스에 맞은 물체: %s ==="), *ClickedActor->GetName());

		// 클릭한 액터가 '현재 공격 가능한 적 목록'에 있는지 인덱스를 찾습니다.
		int32 FoundIndex = AvailableTargets.IndexOfByKey(ClickedActor);

		// 찾았다면? (적을 클릭한 게 맞다면)
		if (FoundIndex != INDEX_NONE)
		{
			// [전체 공격(All)] 이라면 -> 누굴 클릭하든 묻지도 따지지도 않고 발동!
			if (CurrentTargetingType == ETargetingType::All)
			{
				ConfirmTargetAndExecute();
				return;
			}

			// [광역(Area) / 단일 / 랜덤] 이라면
			if (FoundIndex == CurrentTargetIndex)
			{
				// 이미 주 타겟인 녀석을 또 클릭했다면 -> 스킬 발동 확정!
				ConfirmTargetAndExecute();
			}
			else
			{
				// 다른 녀석을 클릭했다면 -> 주 타겟을 그 녀석으로 변경!
				HighlightCurrentTarget(false);
				CurrentTargetIndex = FoundIndex;
				HighlightCurrentTarget(true);

				UE_LOG(LogTemp, Log, TEXT("마우스 타겟 변경: [%d] %s"), CurrentTargetIndex, *ClickedActor->GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. 마우스 아래에 아무것도 없음 (허공 클릭)"));
	}
}

void ASPGASPlayerController::OnBattlePointChanged(const FOnAttributeChangeData& Data)
{
	RefreshBattlePointUI();
}

void ASPGASPlayerController::OnMaxBattlePointChanged(const FOnAttributeChangeData& Data)
{
	RefreshBattlePointUI();
}


void ASPGASPlayerController::ProcessWeaponSwitch(FGameplayTag NewWeaponTag)
{
	if (CurrentWeaponTag == NewWeaponTag) return;
	if (!CachedASC) return;

	CachedASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.Fenrir"));
	CachedASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.Surtr"));
	CachedASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("Weapon.Jormungandr"));

	CachedASC->AddLooseGameplayTag(NewWeaponTag);

	CurrentWeaponTag = NewWeaponTag;
	CurrentSelectedAction = ESelectedActionType::None; // 무기 바뀌면 행동 리셋

	UE_LOG(LogTemp, Log, TEXT("무기 교체 완료: %s"), *NewWeaponTag.ToString());

	// TODO: 캐릭터에게 무기 외형 변경 요청
	// Character->EquipWeapon(NewWeaponTag);
}

bool ASPGASPlayerController::IsMyTurn() const
{
	if (CachedASC)
	{
		return CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Battle_TurnActive);
	}
	return false;
}

void ASPGASPlayerController::OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!IsLocalController()) return;

	// 1. 캐릭터 & 서브시스템 가져오기
	ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn());
	auto* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	
	if (!PlayerChar || !Subsystem) return;

	// 2. NewCount가 0보다 크면 전투 모드
	if (NewCount > 0)
	{
		// [전투 진입]
		Subsystem->RemoveMappingContext(FieldMappingContext);
		Subsystem->AddMappingContext(BattleMappingContext, 0);
		PlayerChar->SetCameraProfile(PlayerChar->GetCombatCameraProfile());
		


		UE_LOG(LogTemp, Warning, TEXT("상태 적용: BATTLE Mode"));
	}
	else
	{
		// [전투 종료/필드]
		Subsystem->RemoveMappingContext(BattleMappingContext);
		Subsystem->AddMappingContext(FieldMappingContext, 0);
		CancelTargetSelection();

		PlayerChar->SetCameraProfile(PlayerChar->GetFieldCameraProfile());

		UE_LOG(LogTemp, Warning, TEXT("상태 적용: FIELD Mode"));
	}
}