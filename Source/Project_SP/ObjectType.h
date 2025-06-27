// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectType.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EFaction : uint8
{
	Player UMETA(DisplayName = "Player"),
	Enemy UMETA(DisplayName = "Enemy"),
	NPC UMETA(DisplayName = "NPC")
};

UCLASS()
class PROJECT_SP_API UObjectType : public UObject
{
	GENERATED_BODY()
	
};
