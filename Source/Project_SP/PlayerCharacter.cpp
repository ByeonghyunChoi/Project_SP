// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	FieldModeComp = CreateDefaultSubobject<UFieldModeComponent>(TEXT("FieldModeComponent"));

	FieldModeComp->SetComponentTickEnabled(false);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void APlayerCharacter::EnterFieldMode()
{
	// 필드 모드 컴포넌트 활성화
	if (FieldModeComp)
	{
		FieldModeComp->SetComponentTickEnabled(true);
		UE_LOG(LogTemp, Log, TEXT("플레이어 필드 모드 진입."));
	}
}

void APlayerCharacter::EnterBattleMode()
{
	// 전투 진입 시 필드 모드 컴포넌트 비활성화
	if (FieldModeComp)
	{
		FieldModeComp->SetComponentTickEnabled(false);
		UE_LOG(LogTemp, Log, TEXT("필드 모드 비활성화."));
	}
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CombatData->SetFaction(EFaction::Player);
}