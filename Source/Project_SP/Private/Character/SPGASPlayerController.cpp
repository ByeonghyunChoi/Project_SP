// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/SPGASPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
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
#include "Game/ASPCombatGameMode.h"
#include "Component/InventoryComponent.h" // 인벤토리 컴포넌트 추가
#include <AbilitySystemBlueprintLibrary.h>
#include "Data/ShopDataStructs.h"
#include "Data/Asset/OpartsDefinition.h"
#include "Component/OpartsComponent.h"
#include "Component/SPTutorialManagerComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"

ASPGASPlayerController::ASPGASPlayerController()
{
	bShowMouseCursor = true;
	TutorialManager = CreateDefaultSubobject<USPTutorialManagerComponent>(TEXT("TutorialManager"));
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

	if (LobbyHUDClass && !LobbyHUDWidget)
	{
		LobbyHUDWidget = CreateWidget<UUserWidget>(this, LobbyHUDClass);
		if (LobbyHUDWidget) LobbyHUDWidget->AddToViewport();
	}
	if (FieldHUDClass && !FieldHUDWidget)
	{
		FieldHUDWidget = CreateWidget<UUserWidget>(this, FieldHUDClass);
		if (FieldHUDWidget) FieldHUDWidget->AddToViewport();
	}
	if (BattleHUDClass && !BattleHUDWidget)
	{
		BattleHUDWidget = CreateWidget<UUserWidget>(this, BattleHUDClass);
		if (BattleHUDWidget) BattleHUDWidget->AddToViewport();
	}

	// 2. 선생님의 MapManager를 이용해 지금 무슨 맵인지 판단합니다.
	FName StartMode = TEXT("Field");
	UGameInstance* GI = GetGameInstance();
	if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
	{
		if (MapManager->IsInBattleMap())
		{
			StartMode = TEXT("Battle");
		}
		else if (MapManager->GetIsInLobby())
		{
			StartMode = TEXT("Lobby");
		}
		else
		{
			StartMode = TEXT("Field");
		}
	}

	// 3. 결정된 UI 딱 하나만 켭니다.
	SwitchHUDMode(StartMode);
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

		// 패링 액션 바인딩
		if (ParryAction)
		{
			EIC->BindAction(ParryAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnParryPressed);
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
		FGameplayTag TimeInterferenceTag = FSPGameplayTags::Get().State_TimeInterference;

		// 이벤트 등록 (앞으로의 변화를 감지하기 위해 등록)
		CachedASC->RegisterGameplayTagEvent(BattleTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerController::OnBattleTagChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetBattlePointAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnBattlePointChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetMaxBattlePointAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnMaxBattlePointChanged);
		CachedASC->RegisterGameplayTagEvent(TimeInterferenceTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerController::OnTimeInterferenceTagChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetExperienceAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnExperienceAttributeChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetLevelAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnLevelAttributeChanged);
		CachedASC->GetGameplayAttributeValueChangeDelegate(USPGASAttributeSet::GetExperienceAttribute())
			.AddUObject(this, &ASPGASPlayerController::OnExperienceAttributeChanged);
		CachedASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_Battle_TurnActive, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerController::OnTurnActiveTagChanged);

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
	if (!bIsSelectingTarget || AvailableTargets.IsEmpty()) return;

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
	if (Direction < 0) // 오른쪽 (D)
	{
		CurrentTargetIndex = (CurrentTargetIndex + 1) % AvailableTargets.Num();
	}
	else // 왼쪽 (A)
	{
		// 음수 모듈러 연산 보정
		CurrentTargetIndex = (CurrentTargetIndex - 1 + AvailableTargets.Num()) % AvailableTargets.Num();
	}

	if (AvailableTargets.IsValidIndex(CurrentTargetIndex) && AvailableTargets[CurrentTargetIndex].IsValid())
	{
		AActor* SelectedTarget = AvailableTargets[CurrentTargetIndex].Get();
		HighlightCurrentTarget(true);
		OnTargetChanged.Broadcast(SelectedTarget);

		// 🌟 [추가] 타겟이 바뀌었으니, 새 타겟 인덱스에 맞춰 카메라를 돌려라!
		if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
		{
			PlayerChar->ToggleActionCameraMode(true, false, CurrentTargetIndex, CurrentTargetingType);
		}
	}
	else
	{
		AvailableTargets.RemoveAt(CurrentTargetIndex);
		OnBattleNavigate(0);
	}
}

void ASPGASPlayerController::OnFieldInputPressed(FGameplayTag InputTag)
{
	if (!CachedASC) return;

	if (InputTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Weapon"))))
	{
		if (CurrentWeaponTag == InputTag)
		{
			PlayActionSound(InputTag, false);
			UE_LOG(LogTemp, Log, TEXT("[필드] 이미 장착 중인 무기입니다."));
			return;
		}

		// 필드에서도 무기 교체 함수를 동일하게 실행!
		PlayActionSound(InputTag, true);
		ProcessWeaponSwitch(InputTag);
		return;
	}

	// 필드는 즉시 실행
	CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}

void ASPGASPlayerController::OnBattleInputPressed(FGameplayTag InputTag)
{
	const FSPGameplayTags& GameplayTags = FSPGameplayTags::Get();

	if (TutorialManager && TutorialManager->IsTutorialActive())
	{
		if (TutorialManager->CanProcessInput(InputTag))
		{
			int32 Step = TutorialManager->GetCurrentStep();

			// [기믹 1] 일반 공격 설명 중 정답 입력 시
			if (Step == 4 && InputTag.MatchesTagExact(GameplayTags.Battle_Action_Attack))
			{
				UGameplayStatics::SetGamePaused(GetWorld(), false); // 세상 정지 해제
				TutorialManager->HideTutorialPopup();              // UI 숨기기
			}
			// [기믹 2] 무기 스킬 설명 중 정답 입력 시
			else if (Step == 6 && InputTag.MatchesTagExact(GameplayTags.Battle_Action_Skill))
			{
				UGameplayStatics::SetGamePaused(GetWorld(), false);
				TutorialManager->HideTutorialPopup();
			}
			// [기믹 3] 시간 간섭 설명 중 정답 입력 시 ➡️ 대단원의 막을 내림!
			else if (Step == 7 && InputTag.MatchesTagExact(GameplayTags.Battle_Action_TimeInterference))
			{
				UGameplayStatics::SetGamePaused(GetWorld(), false);
				TutorialManager->EndTutorial();                    // 튜토리얼 완전 종료 및 자유전투 전환!
			}
		}
		else
		{
			HandleInputFeedback(InputTag, false);
			return; // 오답 키는 철저하게 차단
		}
	}

	// 무기 교체 입력 
	if (InputTag.MatchesTag(FGameplayTag::RequestGameplayTag("Weapon")))
	{
		if (CurrentWeaponTag == InputTag)
		{
			PlayActionSound(InputTag, false);
			HandleInputFeedback(InputTag, false);
			UE_LOG(LogTemp, Log, TEXT("이미 장착 중인 무기입니다. 입력을 무시합니다."));
			return;
		}

		PlayActionSound(InputTag, true);
		HandleInputFeedback(InputTag, true);
		ProcessWeaponSwitch(InputTag);
		return;
	}

	// 턴 체크
	if (!IsMyTurn() && (!TutorialManager || !TutorialManager->IsTutorialActive()))
	{
		UE_LOG(LogTemp, Warning, TEXT("아직 내 턴이 아닙니다."));
		return;
	}
	
	if (InputTag.MatchesTag(GameplayTags.Battle_Action_TimeInterference))
	{
		if (bIsSelectingTarget) CancelTargetSelection(); // 타겟팅 중이었다면 취소

		// 이미 시간 간섭 상태인지 확인 (중복 발동 방지)
		if (CachedASC && CachedASC->HasMatchingGameplayTag(GameplayTags.State_TimeInterference))
		{
			UE_LOG(LogTemp, Warning, TEXT("[시스템] 이미 시간 간섭이 발동 중입니다!"));
			return;
		}

		// 쿨타임 검사 
		if (CachedASC && CachedASC->HasMatchingGameplayTag(GameplayTags.Cooldown_Skill_TimeInterference))
		{
			PlayActionSound(InputTag, false);
			HandleInputFeedback(InputTag, false);
			UE_LOG(LogTemp, Warning, TEXT("[시스템] 시간 간섭 스킬이 쿨타임 중입니다! (남은 턴 대기)"));
			return;
		}

		int32 Cost = GetSkillCost(InputTag);
		if (GetCurrentTimePower() < Cost)
		{
			PlayActionSound(InputTag, false);
			HandleInputFeedback(InputTag, false);
			UE_LOG(LogTemp, Warning, TEXT("[시스템] 시간의 힘이 부족하여 발동할 수 없습니다!"));
			return;
		}

		// 시간 간섭 GA 실행 시도
		if (CachedASC)
		{
			bool bSuccess = CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
			if (bSuccess)
			{
				PlayActionSound(InputTag, true);
				HandleInputFeedback(InputTag, true);
				OnTimeInterferenceChanged.Broadcast(true);

				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭] 발동!"));

				if (TutorialManager && TutorialManager->GetCurrentStep() == 7)
				{
					TutorialManager->AdvanceStep(); // Step 8로 넘어감 (튜토리얼 완전 종료!)
				}
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

	if (InputType != ESelectedActionType::None)
	{
		// 무기 미착용 체크
		if (!CurrentWeaponTag.IsValid())
		{
			PlayActionSound(InputTag, false);
			HandleInputFeedback(InputTag, false);
			UE_LOG(LogTemp, Warning, TEXT("무기를 먼저 선택하세요!"));
			return;
		}

		if (InputType == ESelectedActionType::WeaponSkill)
		{
			// '해골 수정(CrystalSkull)' 고유 태그를 검사합니다!
			bool bIsCrystalSkull = CachedASC && CachedASC->HasMatchingGameplayTag(GameplayTags.State_Buff_CrystalSkull);

			// 1. 쿨타임 검사
			if (!bIsCrystalSkull && GetSkillCooldownTurns(GameplayTags.Battle_Action_Skill) > 0)
			{
				PlayActionSound(InputTag, false);
				HandleInputFeedback(InputTag, false);
				UE_LOG(LogTemp, Warning, TEXT("[시스템] 무기 스킬 쿨타임 중입니다!"));
				return; // 타겟팅 진입 차단!
			}

			// 2. BP 및 프리패스(시간 간섭) 검사
			int32 Cost = GetSkillCost(GameplayTags.Battle_Action_Skill);

			// 해골 수정 버프가 없는데, BP마저 부족하다면?
			if (!bIsCrystalSkull && GetCurrentBP() < Cost)
			{
				PlayActionSound(InputTag, false);
				HandleInputFeedback(InputTag, false);
				UE_LOG(LogTemp, Warning, TEXT("[시스템] BP가 부족합니다!"));
				return; // 타겟팅 진입 차단!
			}
		}

		if (CachedASC && CachedASC->HasMatchingGameplayTag(GameplayTags.State_Buff_CrystalSkull))
		{
			// 선택한 행동이 '무기 스킬'이 아니라면? (일반 공격이나 패링이라면)
			if (InputType != ESelectedActionType::WeaponSkill)
			{
				PlayActionSound(InputTag, false);
				HandleInputFeedback(InputTag, false);
				UE_LOG(LogTemp, Warning, TEXT("[시간 간섭 발동 중!] 무기 스킬과 무기 교체만 사용할 수 있습니다."));
				// 행동을 무시하고 함수 종료 (타겟팅으로 안 넘어감)
				return;
			}
		}

		PlayActionSound(InputTag, true);
		HandleInputFeedback(InputTag, true);

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
			if (bIsSelectingTarget)
			{
				// 🌟 1. 이미 타겟팅 중이었다면 타겟을 유지합니다!
				HighlightCurrentTarget(false); // 잠시 기존 불빛 끄기

				SetCurrentSelectedAction(InputType); // 행동 상태 변경

				// (주의: CurrentTargetingType은 이 코드 직전에 이미 새 행동에 맞춰 갱신되어 있습니다!)
				HighlightCurrentTarget(true);  // 바뀐 규칙(단일/광역)으로 불빛 다시 켜기

				// 🌟 행동이 바뀌면 UI에 뜨는 예상 수치나 설명이 달라질 수 있으므로 방송을 한 번 쏴줍니다!
				if (AvailableTargets.IsValidIndex(CurrentTargetIndex) && AvailableTargets[CurrentTargetIndex].IsValid())
				{
					OnTargetChanged.Broadcast(AvailableTargets[CurrentTargetIndex].Get());
				}

				UE_LOG(LogTemp, Log, TEXT("행동 변경됨: %d -> 기존 타겟[%d] 유지"), (int32)InputType, CurrentTargetIndex);
			}
			else
			{
				// 🌟 2. 타겟팅 중이 아니었다면 처음부터 시작합니다. (이때만 0번으로 리셋)
				SetCurrentSelectedAction(InputType);
				StartTargetSelection();
			}
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

int32 ASPGASPlayerController::GetCurrentPlayerLevel() const
{
	if (!CachedASC) return 1; 

	float CurrentLevel = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetLevelAttribute());
	return FMath::FloorToInt(CurrentLevel);
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

	SwitchHUDMode(TEXT("Battle"));
	UE_LOG(LogTemp, Warning, TEXT("[UI 통합 제어] 전투 화면 UI 및 위젯 컴포넌트 활성화 완료!"));
}

void ASPGASPlayerController::HideBattleUIAndShowFieldUI()
{
	if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
	{
		if (PlayerChar->GetWeaponWidgetComponent()) PlayerChar->GetWeaponWidgetComponent()->SetVisibility(false);
		if (PlayerChar->GetActionWidgetComponent()) PlayerChar->GetActionWidgetComponent()->SetVisibility(false);
		if (PlayerChar->GetBattlePointWidgetComponent()) PlayerChar->GetBattlePointWidgetComponent()->SetVisibility(false);
	}
	CancelTargetSelection();

	// 🌟 필드 UI 켜라!
	SwitchHUDMode(TEXT("Field"));

	UE_LOG(LogTemp, Warning, TEXT("[UI 통합 제어] 필드 UI 전환 및 전투 위젯 컴포넌트 비활성화 완료!"));
}

void ASPGASPlayerController::RefreshBattlePointUI()
{
	if (!CachedASC) return;

	int32 CurrentBP = FMath::FloorToInt(CachedASC->GetNumericAttribute(USPGASAttributeSet::GetBattlePointAttribute()));
	int32 MaxBP = FMath::FloorToInt(CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxBattlePointAttribute()));
	OnBattlePointUIUpdated.Broadcast(CurrentBP, MaxBP);
}


bool ASPGASPlayerController::BuyShopItem(const FShopItemRow& ItemData)
{
	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn) return false;
	
	UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();

	if (InventoryComp)
	{
		// (1) 재고 검사: 0개면 구매 불가! (-1은 무한이므로 통과)
		if (ItemData.Stock == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("재고가 부족하여 %s을(를) 구매할 수 없습니다."), *ItemData.DisplayName.ToString());
			return false;
		}

		// (2) 골드 검사: 돈 없으면 구매 불가!
		if (InventoryComp->GetMoney() < ItemData.Price)
		{
			UE_LOG(LogTemp, Warning, TEXT("골드가 부족하여 %s을(를) 구매할 수 없습니다. (필요: %d)"), *ItemData.DisplayName.ToString(), ItemData.Price);
			return false;
		}

		// (3) 골드 차감! (재고 차감은 여기서 하지 않습니다. UI에서 처리)
		InventoryComp->ConsumeMoney(ItemData.Price);
		UE_LOG(LogTemp, Warning, TEXT("%d 골드를 지불했습니다. 상점 아이템 구매 성공!"), ItemData.Price);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ 플레이어에게서 인벤토리 컴포넌트를 찾을 수 없습니다!"));
		return false;
	}
	

	// 2. 이 아이템에 이펙트(EffectClass)가 설정되어 있다면? (ex) 회복약 처리)
	if (ItemData.EffectClass)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn);
		if (ASC)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemData.EffectClass, 1.0f, Context);

			if (SpecHandle.IsValid())
			{
				// 🌟 [수정된 핵심 1] 플레이어의 현재 '최대 체력(MaxHP)'을 가져옵니다.
				float MaxHP = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());

				// 🌟 [수정된 핵심 2] 데이터 테이블의 수치를 '퍼센트'로 계산합니다.
				// (예: MaxHP 1000 * (30.0 / 100.0) = 300.0)
				float FinalHealAmount = MaxHP * (ItemData.ValueAmount / 100.0f);

				// 🌟 [수정된 핵심 3] 원래의 ValueAmount 대신, 방금 계산한 FinalHealAmount를 주입합니다!
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.HealAmount")), FinalHealAmount);

				// 내 몸에 약 주사!
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

				// 로그도 보기 좋게 변경해줍니다.
				UE_LOG(LogTemp, Warning, TEXT("구매 성공! %s 사용됨 (비율: %f%% / 실제 회복량: %f)"), *ItemData.DisplayName.ToString(), ItemData.ValueAmount, FinalHealAmount);
				return true;
			}
		}
	}

	return true;
}

void ASPGASPlayerController::RefreshExpUI()
{
	if (!CachedASC) return;

	float CurrentExp = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetExperienceAttribute());
	float MaxExp = CachedASC->GetNumericAttribute(USPGASAttributeSet::GetMaxExperienceAttribute());

	// 0 나누기 에러 방지 및 퍼센트 계산 (0.0 ~ 1.0)
	float Percent = (MaxExp > 0.0f) ? (CurrentExp / MaxExp) : 0.0f;

	// UI로 방송!
	OnPlayerExpChanged.Broadcast(CurrentExp, MaxExp, Percent);
}

const UOpartsDefinition* ASPGASPlayerController::GetCurrentOpartsDefinition() const
{
	if (APawn* PlayerPawn = GetPawn())
	{
		if (UOpartsComponent* OpartsComp = PlayerPawn->FindComponentByClass<UOpartsComponent>())
		{
			return OpartsComp->GetCurrentOpartsData().Definition;
		}
	}
	return nullptr;
}

void ASPGASPlayerController::SetAllHUDVisibility(bool bIsVisible)
{
	if (bIsVisible)
	{
		// 🌟 다시 켤 때는 그냥 무식하게 켜지 말고, 현재 맵 상태를 물어봐서 똑똑하게 켭니다!
		UGameInstance* GI = GetGameInstance();
		if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
		{
			if (MapManager->IsInBattleMap()) SwitchHUDMode(TEXT("Battle"));
			else if (MapManager->GetIsInLobby()) SwitchHUDMode(TEXT("Lobby"));
			else SwitchHUDMode(TEXT("Field"));
		}
	}
	else
	{
		// 🌟 끌 때는 3개 다 꺼버림
		if (LobbyHUDWidget) LobbyHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		if (FieldHUDWidget) FieldHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		if (BattleHUDWidget) BattleHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	UE_LOG(LogTemp, Log, TEXT("[UI Control] 컷신 HUD 가시성 변경 완료 -> %s"), bIsVisible ? TEXT("ON") : TEXT("OFF"));
}

void ASPGASPlayerController::SwitchHUDMode(FName ModeName)
{
	// 1. 무조건 3개를 싹 다 끕니다. (중첩 원천 차단)
	if (LobbyHUDWidget) LobbyHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	if (FieldHUDWidget) FieldHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	if (BattleHUDWidget) BattleHUDWidget->SetVisibility(ESlateVisibility::Hidden);

	// 2. 들어온 이름에 맞춰서 딱 하나만 살립니다.
	if (ModeName == TEXT("Lobby") && LobbyHUDWidget)
	{
		LobbyHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ModeName == TEXT("Field") && FieldHUDWidget)
	{
		FieldHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ModeName == TEXT("Battle") && BattleHUDWidget)
	{
		BattleHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ASPGASPlayerController::StartRegressionSequence()
{
	// 1. 화면에 켜져 있던 모든 전투/필드 UI를 강제로 끕니다.
	SetAllHUDVisibility(false);

	// 2. 블루프린트에 만들어둔 '회귀 텍스트 UI'를 띄워라! (이벤트 송출)
	ShowRegressionUI();

	// 3. 로비로 보내버리는 타이머 가동!
	GetWorld()->GetTimerManager().SetTimer(RegressionTimerHandle, this, &ASPGASPlayerController::ExecuteGoToLobby, 5.0f, false);
}

bool ASPGASPlayerController::IsNormalAttackRestricted() const
{
	// 1. 수정 해골 버프가 켜져 있다면 일반 공격 제한!
	if (CachedASC && CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Buff_CrystalSkull))
	{
		return true;
	}
	return false;
}

void ASPGASPlayerController::Cheat_AddAllResources()
{
	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn) return;

	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		// 재화 10000개씩 팍팍 꽂아주기!
		InventoryComp->AddMoney(10000);
		InventoryComp->AddSand(10000);
		InventoryComp->AddIncompleteEnergy(10000);
		InventoryComp->AddFragment(10000);

		// 획득한 상태로 바로 세이브를 구워버려서 맵을 이동해도 안 날아가게 만듭니다.
		if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
		{
			SaveSys->CacheRunDataFromPlayer(PlayerPawn);
			SaveSys->SaveRunToDisk();
			SaveSys->CachePermDataFromPlayer(PlayerPawn);
			SaveSys->SavePermToDisk();
		}
	}
}

void ASPGASPlayerController::Cheat_GoToBoss()
{
	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		MapManager->Cheat_JumpToBossRoom();
	}
}

void ASPGASPlayerController::OnBattleTransitionFinished()
{
	UGameInstance* GI = GetGameInstance();
	if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
	{
		// 연출이 끝났으니 진짜로 전투 레벨을 로드하라고 맵 매니저에게 명령합니다!
		MapManager->ExecuteBattleLevelLoad();
	}
}

void ASPGASPlayerController::OnPortalTransitionFinished()
{
	UGameInstance* GI = GetGameInstance();
	if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
	{
		// 연출이 끝났으니 진짜로 다음 맵 스폰을 실행합니다!
		MapManager->ExecutePortalTransitionLoad();
	}
}

void ASPGASPlayerController::StartTargetSelection()
{
	// 1. 적 목록 찾기
	AvailableTargets.Empty();

	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		TArray<AActor*> BattleEnemies = GM->GetCurrentEnemies();

		for (AActor* Actor : BattleEnemies)
		{
			if (IsValid(Actor))
			{
				AvailableTargets.Add(Actor);
			}
		}
	}

	if (AvailableTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("공격 가능한 적이 없습니다!"));
		CancelTargetSelection();
		return;
	}

	// =========================================================================
	// 🌟 [핵심 추가] 타겟 배열을 '화면 왼쪽에서 오른쪽' 순서로 강제 정렬합니다!
	// =========================================================================
	if (PlayerCameraManager)
	{
		// 카메라가 바라보는 기준에서 완벽한 '오른쪽' 방향 벡터를 구합니다.
		FVector CameraRight = PlayerCameraManager->GetCameraRotation().RotateVector(FVector::RightVector);

		// 람다 함수를 이용해 위치를 비교하여 배열을 정렬합니다.
		AvailableTargets.Sort([CameraRight](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B)
			{
				if (A.IsValid() && B.IsValid())
				{
					// 내적(Dot Product)을 구하면, 몬스터가 화면 좌/우 축의 어느 좌표에 있는지 알 수 있습니다!
					float PositionA = FVector::DotProduct(A->GetActorLocation(), CameraRight);
					float PositionB = FVector::DotProduct(B->GetActorLocation(), CameraRight);

					// 값이 작을수록 화면 왼쪽, 클수록 화면 오른쪽입니다.
					return PositionA < PositionB;
				}
				return false;
			});
	}
	// =========================================================================

	// 2. 초기화
	bIsSelectingTarget = true;
	CurrentTargetIndex = 0;

	// 3. 하이라이트 ON
	HighlightCurrentTarget(true);

	if (AvailableTargets.IsValidIndex(CurrentTargetIndex) && AvailableTargets[CurrentTargetIndex].IsValid())
	{
		OnTargetChanged.Broadcast(AvailableTargets[CurrentTargetIndex].Get());
	}
}

void ASPGASPlayerController::ConfirmTargetAndExecute()
{
	if (AvailableTargets.IsValidIndex(CurrentTargetIndex) && AvailableTargets[CurrentTargetIndex].IsValid())
	{
		AActor* SelectedTarget = AvailableTargets[CurrentTargetIndex].Get();

		// 하이라이트 OFF 및 상태 리셋
		HighlightCurrentTarget(false);
		bIsSelectingTarget = false;

		// 실행 명령
		ExecuteBattleAbility(CurrentSelectedAction, SelectedTarget);

		// 행동 초기화 (다음 턴을 위해)
		SetCurrentSelectedAction(ESelectedActionType::None, true);

		OnTargetChanged.Broadcast(nullptr);
	}
	else
	{
		// 🌟 3. (옵션) 타겟을 확정하려 했으나 이미 파괴된 경우의 예외 처리
		UE_LOG(LogTemp, Warning, TEXT("타겟 확정 실패: 타겟이 이미 파괴되었거나 유효하지 않습니다."));

		// 꼬임을 방지하기 위해 타겟팅 상태를 강제로 초기화
		HighlightCurrentTarget(false);
		bIsSelectingTarget = false;
		SetCurrentSelectedAction(ESelectedActionType::None);

		StartTargetSelection();
	}
}

void ASPGASPlayerController::CancelTargetSelection()
{
	HighlightCurrentTarget(false);
	bIsSelectingTarget = false;
	SetCurrentSelectedAction(ESelectedActionType::None, false);
	AvailableTargets.Empty();
	UE_LOG(LogTemp, Log, TEXT("타겟 선택 취소됨"));
	OnTargetChanged.Broadcast(nullptr);
}

void ASPGASPlayerController::HighlightCurrentTarget(bool bHighlight)
{
	if (AvailableTargets.Num() == 0) return;

	// 1. [전체 공격(All)]
	if (CurrentTargetingType == ETargetingType::All)
	{
		for (int32 i = 0; i < AvailableTargets.Num(); ++i)
		{
			if (AvailableTargets[i].IsValid())
			{
				if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(AvailableTargets[i].Get()))
				{
					// 마커는 전부 크게(true)! 하지만 메인 전광판은 0번만(i==0)!
					Monster->SetSelectedWidget(bHighlight, true, (i == 0));
				}
			}
		}
	}
	// 2. [광역 공격(Area)]
	else if (CurrentTargetingType == ETargetingType::Area)
	{
		for (int32 i = 0; i < AvailableTargets.Num(); ++i)
		{
			if (AvailableTargets[i].IsValid())
			{
				if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(AvailableTargets[i].Get()))
				{
					bool bIsPrimary = (i == CurrentTargetIndex);
					// 내가 선택한 놈만 마커도 크게, 전광판도 띄움!
					Monster->SetSelectedWidget(bHighlight, bIsPrimary, bIsPrimary);
				}
			}
		}
	}
	// 3. [단일 공격(Single)]
	else
	{
		if (AvailableTargets.IsValidIndex(CurrentTargetIndex) && AvailableTargets[CurrentTargetIndex].IsValid())
		{
			if (ASPGASMonsterCharacter* Monster = Cast<ASPGASMonsterCharacter>(AvailableTargets[CurrentTargetIndex].Get()))
			{
				// 한 놈이니까 무조건 둘 다 true!
				Monster->SetSelectedWidget(bHighlight, true, true);
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
		
		if (TutorialManager && TutorialManager->IsTutorialActive())
		{
			// 일반 공격 발사 성공 -> 이제 적의 턴이 오고, 패링 타이밍(Step 5)을 기다림
			if (TutorialManager->GetCurrentStep() == 4 && ActionType == ESelectedActionType::NormalAttack)
			{
				TutorialManager->SetCurrentStep(5);
			}
			// 무기 스킬 발사 성공 -> 이제 턴이 한 바퀴 돌고 시간 간섭(Step 7)을 기다림
			else if (TutorialManager->GetCurrentStep() == 6 && ActionType == ESelectedActionType::WeaponSkill)
			{
				TutorialManager->SetCurrentStep(7);
			}
		}
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
		if (!IsValid(ClickedActor))
		{
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("=== 4. 마우스에 맞은 물체: %s ==="), *ClickedActor->GetName());

		// 클릭한 액터가 '현재 공격 가능한 적 목록'에 있는지 인덱스를 찾습니다.
		int32 FoundIndex = INDEX_NONE;
		for (int32 i = 0; i < AvailableTargets.Num(); ++i)
		{
			if (AvailableTargets[i].IsValid() && AvailableTargets[i].Get() == ClickedActor)
			{
				FoundIndex = i;
				break;
			}
		}

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
				if (AvailableTargets.IsValidIndex(CurrentTargetIndex) && AvailableTargets[CurrentTargetIndex].IsValid())
				{
					// 이미 주 타겟인 녀석을 또 클릭했다면 -> 스킬 발동 확정!
					ConfirmTargetAndExecute();
				}
			}
			else
			{
				// 다른 녀석을 클릭했다면 -> 주 타겟을 그 녀석으로 변경!
				HighlightCurrentTarget(false);
				CurrentTargetIndex = FoundIndex;
				HighlightCurrentTarget(true);
				OnTargetChanged.Broadcast(ClickedActor);

				if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
				{
					PlayerChar->ToggleActionCameraMode(true, false, CurrentTargetIndex, CurrentTargetingType);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. 마우스 아래에 아무것도 없음 (허공 클릭)"));
	}
}

void ASPGASPlayerController::OnParryPressed(const FInputActionValue& Value)
{
	if (!CachedASC) return;

	if (TutorialManager && TutorialManager->IsTutorialActive())
	{
		// 🌟 적이 공격해와서 세상이 멈춘 Step 5 상태일 때 패링 키를 눌렀다면?
		if (TutorialManager->GetCurrentStep() == 5)
		{
			UGameplayStatics::SetGamePaused(GetWorld(), false); // 정지 풀기
			TutorialManager->HideTutorialPopup();              // UI 숨기기
			TutorialManager->SetCurrentStep(6);                // 🚨 다음 타겟인 '무기스킬 대기상태'로 변경!

			// ====================================================================
			// 🌟 [핵심 수정] 엔진이 정지를 완전히 풀고 물리/애니메이션 틱을 재개할 수 있도록
			// 0.05초만 딜레이를 준 뒤에 패링 스킬을 발동시킵니다!
			// ====================================================================
			FTimerHandle ParryTriggerTimer;
			GetWorld()->GetTimerManager().SetTimer(
				ParryTriggerTimer,
				[this]()
				{
					if (CachedASC)
					{
						FGameplayTag ParryTag = FSPGameplayTags::Get().Battle_Action_Parry;
						CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ParryTag)); // 패링 발사!
						UE_LOG(LogTemp, Warning, TEXT("[튜토리얼] 지연 패링 발동 완료!"));
					}
				},
				0.05f, false);

			return;
		}
		else if (TutorialManager->GetCurrentStep() < 5)
		{
			return;
		}
	}

	if (IsMyTurn())
	{
		UE_LOG(LogTemp, Warning, TEXT("지금은 내 턴입니다. 패링을 사용할 수 없습니다."));
		return;
	}

	bool bActivated = CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FSPGameplayTags::Get().Battle_Action_Parry));

	UE_LOG(LogTemp, Warning, TEXT("[Input] 실시간 패링 키 눌림! (스킬 발동 시도)"));

}


void ASPGASPlayerController::OnBattlePointChanged(const FOnAttributeChangeData& Data)
{
	RefreshBattlePointUI();
}

void ASPGASPlayerController::OnMaxBattlePointChanged(const FOnAttributeChangeData& Data)
{
	RefreshBattlePointUI();
}

void ASPGASPlayerController::SetCurrentSelectedAction(ESelectedActionType NewAction, bool bInstantReset)
{
	if(CurrentSelectedAction != NewAction)
	{
		CurrentSelectedAction = NewAction;
		OnActionStateChanged.Broadcast(CurrentSelectedAction);
		UE_LOG(LogTemp, Log, TEXT("상태 변경 방송: %d"), (int32)CurrentSelectedAction);

		if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
		{
			bool bIsSelecting = (NewAction != ESelectedActionType::None);
			PlayerChar->ToggleActionCameraMode(bIsSelecting, bInstantReset, CurrentTargetIndex, CurrentTargetingType);
		}
	}
}

void ASPGASPlayerController::OnTimeInterferenceTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!IsLocalController()) return;
	bool bIsActive = (NewCount > 0);

	ToggleTimeInterferenceUI(bIsActive);
	OnTimeInterferenceChanged.Broadcast(bIsActive);
}

void ASPGASPlayerController::OnExperienceAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshExpUI();
}

void ASPGASPlayerController::OnLevelAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (CachedASC)
	{
		int32 CurrentLevel = FMath::FloorToInt(CachedASC->GetNumericAttribute(USPGASAttributeSet::GetLevelAttribute()));
		OnPlayerLevelChanged.Broadcast(CurrentLevel);
	}
}

void ASPGASPlayerController::OnTurnActiveTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!IsLocalController()) return;

	// 내 몸에 TurnActive 태그가 1개 이상 붙었다면 = 내 턴 시작!
	if (NewCount > 0)
	{
		// UI 쪽에 "내 턴 시작됐다! 쿨타임 숫자 다시 그려라!" 라고 방송 송출
		OnPlayerTurnStarted.Broadcast();
	}
}

void ASPGASPlayerController::PlayActionSound(FGameplayTag InputTag, bool bIsSuccess)
{
	for (const FSPInputConfig& Config : BattleInputConfigs)
	{
		// 입력된 태그와 일치하는 세팅을 찾았다면?
		if (Config.InputTag == InputTag)
		{
			// 성공 여부에 따라 재생할 소리를 결정
			USoundBase* SoundToPlay = bIsSuccess ? Config.ValidSound : Config.InvalidSound;

			if (SoundToPlay)
			{
				UGameplayStatics::PlaySound2D(this, SoundToPlay);
			}
			return; // 소리를 틀었으니 종료
		}
	}
}

void ASPGASPlayerController::HandleInputFeedback(FGameplayTag InputTag, bool bIsSuccess)
{
	OnInputProcessed.Broadcast(InputTag, bIsSuccess);
}

void ASPGASPlayerController::ExecuteGoToLobby()
{
	if (RegressionHUDWidget)
	{
		RegressionHUDWidget->RemoveFromParent(); 
		RegressionHUDWidget = nullptr;       
	}

	UGameInstance* GI = GetGameInstance();
	if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
	{
		// 맵 매니저의 기능 재활용! (세이브 데이터 초기화 + 로비 레벨 이동)
		MapManager->GoToLobby();
	}
}


void ASPGASPlayerController::ProcessWeaponSwitch(FGameplayTag NewWeaponTag)
{
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (CurrentWeaponTag == NewWeaponTag) return;
	if (!CachedASC) return;

	CachedASC->RemoveLooseGameplayTag(SPTags.Weapon_Fenrir);
	CachedASC->RemoveLooseGameplayTag(SPTags.Weapon_Surtr);
	CachedASC->RemoveLooseGameplayTag(SPTags.Weapon_Jormungandr);

	CachedASC->AddLooseGameplayTag(NewWeaponTag);
	CurrentWeaponTag = NewWeaponTag;

	if (ASPGASPlayerCharacter* PlayerChar = Cast<ASPGASPlayerCharacter>(GetPawn()))
	{
		PlayerChar->PlayWeaponSwapSequence(NewWeaponTag);
		OnWeaponChanged.Broadcast(NewWeaponTag);
	}

	UE_LOG(LogTemp, Log, TEXT("무기 교체 완료: %s"), *NewWeaponTag.ToString());

	// 전투 상태일 때 행동 강제 초기화!
	if (CachedASC->HasMatchingGameplayTag(SPTags.State_Mode_Battle))
	{
		// 행동을 고르는 중이었거나, 타겟팅 중이었다면?
		if (bIsSelectingTarget || CurrentSelectedAction != ESelectedActionType::None)
		{
			// 타겟팅 해제 + 상태 None 변경 + 카메라 스무스 복귀 + UI 숨기기
			// 이 모든 것이 CancelTargetSelection() 한 방에 전부 처리됩니다!
			CancelTargetSelection();
			UE_LOG(LogTemp, Log, TEXT("[전투] 무기 교체로 인해 행동 선택이 초기화되었습니다."));
		}
	}
	else if (CachedASC->HasMatchingGameplayTag(SPTags.State_Mode_Field))
	{
		// [필드 상태일 때]
		bIsSelectingTarget = false;
		SetCurrentSelectedAction(ESelectedActionType::None);
		UE_LOG(LogTemp, Log, TEXT("[필드] 무기가 성공적으로 교체되었습니다."));
	}
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
		
		PlayerChar->SwitchCameraMode(true);

		UE_LOG(LogTemp, Warning, TEXT("상태 적용: BATTLE Mode"));
	}
	else
	{
		// [전투 종료/필드]
		Subsystem->RemoveMappingContext(BattleMappingContext);
		Subsystem->AddMappingContext(FieldMappingContext, 0);
		CancelTargetSelection();

		PlayerChar->SwitchCameraMode(false);

		UE_LOG(LogTemp, Warning, TEXT("상태 적용: FIELD Mode"));
	}
}