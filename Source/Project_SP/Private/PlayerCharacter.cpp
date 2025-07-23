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
	if (BattleTurnComponent)
	{
		BattleTurnComponent->SetComponentTickEnabled(false);
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

	if (BattleTurnComponent)
	{
		BattleTurnComponent->SetComponentTickEnabled(true);
	}
}

void APlayerCharacter::PlayerAttackSelectedTarget()
{	

	if (!SelectedTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("공격 대상이 없습니다."));
		return;
	}

	float Damage = StatsComponent->GetAttackPower() - SelectedTarget->GetStatsComponent()->GetDefensePower();
	Damage = FMath::Max(1.0f, Damage);

	float NewHealth = SelectedTarget->GetStatsComponent()->GetCurrentHealth() - Damage;
	SelectedTarget->GetStatsComponent()->SetCurrentHealth(NewHealth);
	UE_LOG(LogTemp, Log, TEXT("%s 가 %s 에게 %f 피해를 입힘"), *GetCharacterName(), *SelectedTarget->GetCharacterName(), Damage);

	BattleTurnComponent->BattleManagerRef->EndTurn();
}

void APlayerCharacter::SelectMonster(ACombatPawn* Target)
{
	SelectedTarget = Target;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Faction = EFaction::Player;
}