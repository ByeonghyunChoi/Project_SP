// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FieldModeComponent.h"
#include "CombatPawn.h"
#include "BattleManager.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// 필드 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UFieldModeComponent* FieldModeComp;

	// 모드 전환 함수
	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterFieldMode();

	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterBattleMode();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void PlayerAttackSelectedTarget();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void SelectMonster(ACombatPawn* target);

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	ACombatPawn* SelectedTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
