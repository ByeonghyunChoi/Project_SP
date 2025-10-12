// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    SetFieldInputMode();
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
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(FieldIMC, 0);
    }
}

void AMyPlayerController::SetPlayerTurnInputMode()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(PlayerTurnIMC, 0);
    }
}

void AMyPlayerController::SetEnemyTurnInputMode()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(EnemyTurnIMC, 0);
    }
}
