// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASPCombatGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API AASPCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

private:
	FTransform GetSpawnTransformByIndex(int32 Index);
};
