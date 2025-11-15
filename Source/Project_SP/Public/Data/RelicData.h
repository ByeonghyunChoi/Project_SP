// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "RelicData.generated.h"

UENUM(BlueprintType)
enum class ERelicType : uint8
{
	None			UMETA(DisplayName = "None"),
	Common		UMETA(DisplayName = "Common"),
	Rare		UMETA(DisplayName = "Rare"),
	Uniqe	UMETA(DisplayName = "Uniqe"),
};

USTRUCT(BlueprintType)
struct FRelicData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	FText RelicName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	ERelicType RelicType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	FText RelicExplain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	TSoftObjectPtr<class UTexture2D> Icon;
};