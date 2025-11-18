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

public:
    // --- 1. 기본 생성자 (Default Constructor) ---
    FRelicData()
        : RelicName(FText::GetEmpty())
        , RelicType(ERelicType::None)
        , RelicExplain(FText::GetEmpty())
        , Icon(nullptr)
    {
    }

    // --- 2. 파라미터 생성자 (Parameterized Constructor) ---
    FRelicData(
        FText InRelicName,
        ERelicType InRelicType,
        FText InRelicExplain,
        TSoftObjectPtr<class UTexture2D> InIcon
    )
        : RelicName(InRelicName)
        , RelicType(InRelicType)
        , RelicExplain(InRelicExplain)
        , Icon(InIcon)
    {
    }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	FText RelicName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	ERelicType RelicType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	FText RelicExplain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic")
	TSoftObjectPtr<class UTexture2D> Icon;
};