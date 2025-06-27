// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCharacterBase* BasicStats;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
