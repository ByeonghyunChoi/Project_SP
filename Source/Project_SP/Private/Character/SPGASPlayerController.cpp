// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Character/SPGASCharacterBase.h"
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

		//필드용 입력 배열 바인딩
		for (const FSPInputConfig& Config : FieldInputConfigs)
		{
			EIC->BindAction(Config.InputAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnInputPressed, Config.InputTag);
		}

		//전투용 입력 배열 바인딩
		for (const FSPInputConfig& Config : BattleInputConfigs)
		{
			EIC->BindAction(Config.InputAction, ETriggerEvent::Started, this, &ASPGASPlayerController::OnInputPressed, Config.InputTag);
		}
	}
}

void ASPGASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ASPGASCharacterBase* SPCharacter = Cast<ASPGASCharacterBase>(InPawn))
	{
		CachedASC = SPCharacter->GetAbilitySystemComponent();
		if (CachedASC)
		{
			CachedASC->RegisterGameplayTagEvent(FSPGameplayTags::Get().State_Mode_Battle, EGameplayTagEventType::AnyCountChange)
				.AddUObject(this, &ASPGASPlayerController::OnBattleTagChanged);
		}
	}

	// 기본은 필드 모드로 시작
	if (auto* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->AddMappingContext(FieldMappingContext, 0);
	}
}

void ASPGASPlayerController::OnMove(const FInputActionValue& Value)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASPGASPlayerController::OnInputPressed(FGameplayTag InputTag)
{
	if (CachedASC)
	{
		// 입력받은 태그를 그대로 ASC에 전달하여 대응하는 어빌리티 실행
		CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
	}
}


void ASPGASPlayerController::OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	auto* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	if (NewCount > 0) // 전투 진입
	{
		Subsystem->RemoveMappingContext(FieldMappingContext);
		Subsystem->AddMappingContext(BattleMappingContext, 0);
	}
	else // 필드 복귀
	{
		Subsystem->RemoveMappingContext(BattleMappingContext);
		Subsystem->AddMappingContext(FieldMappingContext, 0);
	}
}
