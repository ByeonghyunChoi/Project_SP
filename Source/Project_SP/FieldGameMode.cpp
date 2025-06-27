// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldGameMode.h"

AFieldGameMode::AFieldGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> ThirdPersonClassRef(TEXT("/Game/Field/Character/Player/BP_FieldPlayer.BP_FieldPlayer_C"));
	if (ThirdPersonClassRef.Class)
	{
		DefaultPawnClass = ThirdPersonClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Game/Field/Character/Player/BP_FieldPlayerController.BP_FieldPlayerController_C"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}
}

