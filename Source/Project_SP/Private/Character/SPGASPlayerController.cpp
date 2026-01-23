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

void ASPGASPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	// 여기서 클라이언트 컨트롤러가 ASC를 찾아 저장합니다.
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
		UE_LOG(LogTemp, Warning, TEXT("Client Controller: CachedASC Found!"));
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

		// [수정 포인트] GetControlRotation() 대신 PlayerCameraManager를 사용합니다!
		// 이것이 "현재 화면의 방향"을 정확하게 가져옵니다.
		FRotator Rotation = PlayerCameraManager->GetCameraRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 입력값 적용 (Vector.Y = W/S, Vector.X = D/A)
		ControlledPawn->AddMovementInput(ForwardDirection, Vector.Y);
		ControlledPawn->AddMovementInput(RightDirection, Vector.X);
	}
}

void ASPGASPlayerController::OnInputPressed(FGameplayTag InputTag)
{
	if (CachedASC == nullptr)
	{
		// PlayerState가 존재하는지, 그리고 타입이 맞는지 확인
		APlayerState* PS = GetPlayerState<APlayerState>();
		if (PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("Current PlayerState Class: %s"), *PS->GetClass()->GetName());

			ASPGASPlayerState* MyPS = Cast<ASPGASPlayerState>(PS);
			if (!MyPS)
			{
				UE_LOG(LogTemp, Error, TEXT("ERROR: PlayerState is NOT ASPGASPlayerState! Check GameMode!"));
			}
			else
			{
				CachedASC = MyPS->GetAbilitySystemComponent();
				if (!CachedASC)
				{
					UE_LOG(LogTemp, Error, TEXT("ERROR: PlayerState exists, but ASC is NULL! Check Constructor!"));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: PlayerState is NULL!"));
		}
	}

	// [1. 안전장치: CachedASC가 없으면 다시 찾는다]
	if (CachedASC == nullptr)
	{
		// 1-1. 가장 확실한 방법: PlayerState를 직접 뒤져본다.
		if (ASPGASPlayerState* PS = GetPlayerState<ASPGASPlayerState>())
		{
			CachedASC = PS->GetAbilitySystemComponent();
			if (CachedASC)
			{
				UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Found ASC via PlayerState!"));
			}
		}
	}

	// [2. 그래도 없으면 Pawn에게 물어본다 (비상용)]
	if (CachedASC == nullptr)
	{
		if (APawn* MyPawn = GetPawn())
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(MyPawn))
			{
				CachedASC = ASI->GetAbilitySystemComponent();
			}
		}
	}

	// [3. 검증 및 실행]
	if (CachedASC)
	{
		UE_LOG(LogTemp, Log, TEXT("TryActivate: %s"), *InputTag.ToString());
		CachedASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FATAL: Still cannot find ASC! (Check PlayerState replication)"));
	}
}


void ASPGASPlayerController::OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	auto* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem || !CachedASC) return;

	FGameplayTag FieldAbilityTag = FSPGameplayTags::Get().Ability_Type_Field;
	FGameplayTag BattleAbilityTag = FSPGameplayTags::Get().Ability_Type_Battle;
	FGameplayTagContainer FieldAbilities(FieldAbilityTag);
	FGameplayTagContainer BattleAbilities(BattleAbilityTag);

	if (NewCount > 0) // 전투 진입
	{
		Subsystem->RemoveMappingContext(FieldMappingContext);
		Subsystem->AddMappingContext(BattleMappingContext, 0);

		CachedASC->CancelAbilities(&FieldAbilities);
		CachedASC->TryActivateAbilitiesByTag(BattleAbilities);
	}
	else // 필드 복귀
	{
		Subsystem->RemoveMappingContext(BattleMappingContext);
		Subsystem->AddMappingContext(FieldMappingContext, 0);

		CachedASC->CancelAbilities(&BattleAbilities);
		CachedASC->TryActivateAbilitiesByTag(FieldAbilities);
	}
}
