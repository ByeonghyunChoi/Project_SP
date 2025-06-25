// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_SPGameMode.h"
#include "Project_SPPlayerController.h"
#include "Project_SPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_SPGameMode::AProject_SPGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AProject_SPPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}