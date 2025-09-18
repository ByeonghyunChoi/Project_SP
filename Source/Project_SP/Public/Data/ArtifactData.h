// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ArtifactData.generated.h"


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
    // ESOT_StatusAccuracy UMETA(DisplayName = "Status Accuracy"), // 상태적중은 추후 추가
    //ESOT_DefensePenetration UMETA(DisplayName = "Defense Penetration"), // 방어력 무시도 나중에 추가
    ESOT_Attack UMETA(DisplayName = "Attack"),
    ESOT_Defense UMETA(DisplayName = "Defense"),
    ESOT_Health UMETA(DisplayName = "Health"),
    ESOT_Speed UMETA(DisplayName = "Speed")
};

USTRUCT(BlueprintType)
struct FArtifactData : public FTableRowBase
{
    GENERATED_BODY()

    // 아티팩트의 유형
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Data")
    EArtifactType ArtifactType;

    // 아티팩트의 등급
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Data")
    EArtifactGrade ArtifactGrade;

    // 아티팩트가 부여하는 기본 능력치 (체력, 공격력, 방어력, 속도)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float AttackBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float DefenseBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float HealthBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float SpeedBonus = 0.0f;

    // 아티팩트의 특수 옵션 유형과 값
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    ESpecialOptionType SpecialOptionType = ESpecialOptionType::ESOT_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    float SpecialOptionValue = 0.0f;

    // 아티팩트가 속한 세트의 이름을 지정합니다. (세트 효과 구현 시 사용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Artifact Stats")
    FName SetName;
};