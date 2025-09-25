// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextFieldRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Field/Actions/IMC_Field.IMC_Field'"));
    if (nullptr != InputMappingContextFieldRef.Object)
    {
        FieldIMC = InputMappingContextFieldRef.Object;
    }

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextPlayerTurnRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Battle/Actions/IMC_PlayerTurn.IMC_PlayerTurn'"));
    if (nullptr != InputMappingContextPlayerTurnRef.Object)
    {
        PlayerTurnIMC = InputMappingContextPlayerTurnRef.Object;
    }

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextEnemyTurnRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Battle/Actions/IMC_EnemyTurn.IMC_EnemyTurn'"));
    if (nullptr != InputMappingContextEnemyTurnRef.Object)
    {
        EnemyTurnIMC = InputMappingContextEnemyTurnRef.Object;
    }

}

void AMyPlayerController::ShowFieldHUD()
{
    ClearCurrentHUD(); // 기존 HUD 제거
    if (FieldHUDClass)
    {
        CurrentHUD = CreateWidget<UUserWidget>(this, FieldHUDClass);
        if (CurrentHUD)
        {
            CurrentHUD->AddToViewport();
        }
    }
}

void AMyPlayerController::ShowBattleHUD()
{
    ClearCurrentHUD(); // 기존 HUD 제거
    if (BattleHUDClass)
    {
        CurrentHUD = CreateWidget<UUserWidget>(this, BattleHUDClass);
        if (CurrentHUD)
        {
            CurrentHUD->AddToViewport();
        }
    }
}

void AMyPlayerController::ClearCurrentHUD()
{
    if (CurrentHUD)
    {
        CurrentHUD->RemoveFromParent();
        CurrentHUD = nullptr;
    }
}

void AMyPlayerController::SetFieldInputMode()
{
}

void AMyPlayerController::SetPlayerTurnInputMode()
{
}

void AMyPlayerController::SetEnemyTurnInputMode()
{
}
