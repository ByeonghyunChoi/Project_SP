// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.h"
#include "FieldModeComponent.h"
#include "BattleModeComponent.h"
#include "CombatPawn.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CombatData")
	virtual UCharacterBase* GetCombatData() const override { return BasicStats; }

	// 필드 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UFieldModeComponent* FieldModeComp;

	// 전투 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UBattleModeComponent* BattleModeComp;

	// 모드 전환 함수
	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterFieldMode();

	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterBattleMode();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCharacterBase* BasicStats;
};
