// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Character/SPGASCharacterBase.h"
#include "Character/SPGASPlayerState.h"
#include "Tag/SPGameplayTags.h"

ASPGASPlayerController::ASPGASPlayerController()
{
	bShowMouseCursor = true;
}

void ASPGASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputModeData;
	InputModeData.SetHideCursorDuringCapture(false);
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);
}

void ASPGASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		//이동 바인딩
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASPGASPlayerController::OnMove);

		// 필드 입력 바인딩
		for (const FSPInputConfig& Config : FieldInputConfigs)
		{
			if (Config.InputAction && Config.InputTag.IsValid())
			{
				EIC->BindAction(Config.InputAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnInputPressed, Config.InputTag);
			}
		}
		// 전투 입력 바인딩
		for (const FSPInputConfig& Config : BattleInputConfigs)
		{
			if (Config.InputAction && Config.InputTag.IsValid())
			{
				EIC->BindAction(Config.InputAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnInputPressed, Config.InputTag);
			}
		}
	}
}

void ASPGASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitAbilitySystem(InPawn);
}

void ASPGASPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);
	InitAbilitySystem(InPawn);
}

void ASPGASPlayerController::InitAbilitySystem(APawn* InPawn)
{
	if (!InPawn) return;

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
		CachedASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_Mode_Battle, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASPGASPlayerController::OnBattleTagChanged);

		// [중요] 초기 상태 확인 (이미 전투 중인 상태로 빙의했을 수도 있음)
		bool bIsBattle = CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Mode_Battle);

		// 초기 IMC 설정 (로컬 컨트롤러인 경우에만)
		if (IsLocalController())
		{
			if (auto* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->ClearAllMappings();
				if (bIsBattle)
				{
					Subsystem->AddMappingContext(BattleMappingContext, 0);
				}
				else
				{
					Subsystem->AddMappingContext(FieldMappingContext, 0);
				}
			}
		}
	}
}

void ASPGASPlayerController::OnMove(const FInputActionValue& Value)
{
	// 1. 상태 이상(기절 등) 체크
	if (CachedASC && CachedASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Status_BlockMove))
	{
		return;
	}

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

void ASPGASPlayerController::OnInputPressed(FGameplayTag InputTag)
{
	if (!CachedASC) return;
	// 해당 태그를 Trigger로 가진 어빌리티 실행 시도
	CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}


void ASPGASPlayerController::OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!IsLocalController()) return;

	auto* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	FGameplayTag FieldAbilityTag = FSPGameplayTags::Get().Ability_Type_Field;
	FGameplayTag BattleAbilityTag = FSPGameplayTags::Get().Ability_Type_Battle;

	// FGameplayTagContainer는 포인터 말고 값으로 전달하는 것이 안전함
	FGameplayTagContainer FieldAbilities(FieldAbilityTag);
	FGameplayTagContainer BattleAbilities(BattleAbilityTag);

	if (NewCount > 0) // 전투 진입
	{
		// 1. 입력 컨텍스트 교체
		Subsystem->RemoveMappingContext(FieldMappingContext);
		Subsystem->AddMappingContext(BattleMappingContext, 0);

		// 2. 어빌리티 정리 (선택 사항: 로직에 따라 ASC에서 처리하는게 더 좋을 수도 있음)
		// 만약 이 코드가 '입력'만 바꾸는 게 아니라 실제 '어빌리티'도 끄고 켜는 것이라면
		// 서버인 경우에도 실행되어야 함. 하지만 여기서는 IMC 변경과 묶여 있으므로 로컬 처리로 가정.
		if (CachedASC)
		{
			CachedASC->CancelAbilities(&FieldAbilities);
			// 전투 모드 패시브 활성화가 필요하다면 여기서 (하지만 보통 GiveAbilities에서 처리됨)
			// CachedASC->TryActivateAbilitiesByTag(BattleAbilities); 
		}

		UE_LOG(LogTemp, Log, TEXT("⚔️ Controller: Switched to BATTLE IMC"));
	}
	else // 필드 복귀
	{
		Subsystem->RemoveMappingContext(BattleMappingContext);
		Subsystem->AddMappingContext(FieldMappingContext, 0);

		if (CachedASC)
		{
			CachedASC->CancelAbilities(&BattleAbilities);
		}

		UE_LOG(LogTemp, Log, TEXT("🎮 Controller: Switched to FIELD IMC"));
	}
}
