// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldGameMode.h"

AFieldGameMode::AFieldGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> ThirdPersonClassRef(TEXT("/Game/Field/Character/Player/BP_FieldCharacter.BP_FieldCharacter_C"));
	if (ThirdPersonClassRef.Class)
	{
		DefaultPawnClass = ThirdPersonClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Game/Field/Character/Player/BP_FieldPlayerController2.BP_FieldPlayerController2_C"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}
}

