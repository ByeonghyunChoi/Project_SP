// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ArtifactData.generated.h"

UENUM(BlueprintType)
enum class EArtifactKinds : uint8
{
	EAK_Health UMETA(DisplayName = "Health"),
	EAK_Attack UMETA(DisplayName = "Attack"),
	EAK_Defense UMETA(DisplayName = "Defense"),
	EAK_Speed UMETA(DisplayName = "Speed"),
	EAK_Special UMETA(DisplayName = "Special")
};

// 아티팩트의 등급을 정의하는 열거형
UENUM(BlueprintType)
enum class EArtifactGrade : uint8
{
    EAG_Normal UMETA(DisplayName = "Normal"),
    EAG_Rare UMETA(DisplayName = "Rare"),
    EAG_Unique UMETA(DisplayName = "Unique"),
    EAG_Legendary UMETA(DisplayName = "Legendary"),
    EAG_Mythic UMETA(DisplayName = "Mythic")
};

// 아티팩트의 유형을 정의하는 열거형
UENUM(BlueprintType)
enum class EArtifactType : uint8
{
    EAT_Crystal UMETA(DisplayName = "Crystal"),
    EAT_Oxyge UMETA(DisplayName = "Oxyge"),
    EAT_Goldberg UMETA(DisplayName = "Goldberg")
};

// 아티팩트가 가질 수 있는 모든 특수 옵션을 정의
UENUM(BlueprintType)
enum class ESpecialOptionType : uint8
{
    ESOT_None UMETA(DisplayName = "None"),
    ESOT_CritChance UMETA(DisplayName = "Critical Hit Chance"),
    ESOT_CritDamage UMETA(DisplayName = "Critical Hit Damage"),
    ESOT_StatusAccuracy UMETA(DisplayName = "Status Accuracy"),
    ESOT_DefensePenetration UMETA(DisplayName = "Defense Penetration"),
    ESOT_Attack UMETA(DisplayName = "Attack"),
    ESOT_Defense UMETA(DisplayName = "Defense"),
    ESOT_Health UMETA(DisplayName = "Health"),
    ESOT_Speed UMETA(DisplayName = "Speed")
};

USTRUCT(BlueprintType)
struct FArtifactStats : public FTableRowBase // 아티팩트의 기본 능력치 + 등급에 따른 증가치
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	EArtifactKinds ArtifactKinds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float BaseValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float RareMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float UniqueMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float LegendaryMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MythicMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    ESpecialOptionType SpecialOptionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float BaseSpecialValue;
};

USTRUCT(BlueprintType)
struct FArtifactData
{
    GENERATED_BODY()

    // 아티팩트의 종류
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Data")
    EArtifactKinds ArtifactKinds;

    // 아티팩트의 유형
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Data")
    EArtifactType ArtifactType;

	// 아티팩트의 등급
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Data")
    EArtifactGrade ArtifactGrade;

    // 계산된 최종 능력치
    // 아티팩트의 종류에 따라 이 중 하나만 0이 아닌 값을 가진다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Data")
	FArtifactStats ArtifactStats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float FinalAttack = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float FinalDefense = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float FinalHealth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float FinalSpeed = 0.0f;

    // 계산된 최종 특수 옵션 유형과 값
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    ESpecialOptionType SpecialOptionType = ESpecialOptionType::ESOT_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float SpecialOptionValue = 0.0f;
};