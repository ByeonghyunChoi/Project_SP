// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StatusEffectData.generated.h"

/**
 * 
 */
 // 효과의 종류 (버프, 디버프, 지속 데미지 등)
UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
    Stat_Debuff        UMETA(DisplayName = "스탯 디버프"),
    DamageOverTime     UMETA(DisplayName = "지속 데미지"),
    Control_Stun       UMETA(DisplayName = "행동 불가"),
    Damage_Fixed       UMETA(DisplayName = "고정 데미지")
};

// 변경할 스탯의 종류
UENUM(BlueprintType)
enum class EStatToModify : uint8
{
    None,
    CurrentHealth,
    DefensePower,
    MovementSpeed
};

//상태 이상 세부 효과 정보
USTRUCT(BlueprintType)
struct FStatusSubEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStatusEffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStatToModify StatToModify;

    // 효과량 (예: -10%는 -0.1, 20% 데미지는 0.2)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectMagnitude;
};

USTRUCT(BlueprintType)
struct FStatusEffectData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // UI에 표시될 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    // 지속 턴
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TurnDuration;

    // 변경할 스탯 (스탯 버프/디버프일 경우)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStatToModify StatToModify;

    // 조합될 대상 상태이상의 ID 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CombinationTargetID;

    // 조합 성공 시 생성될 상태이상의 ID (예: 화상의 경우 '열풍')
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CombinationResultID;

    // 여러 개의 세부 효과를 담을 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FStatusSubEffect> SubEffects;
};
