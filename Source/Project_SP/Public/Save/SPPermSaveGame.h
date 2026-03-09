// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/SPDataStructs.h"
#include "SPPermSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPPermSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FPlayerMetaProgressionData PermData;
};
