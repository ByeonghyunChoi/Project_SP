#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActionData.generated.h"

class UGameAction;

/**
 *
 */

UENUM(BlueprintType)
enum class ETargetingType : uint8
{
    Self UMETA(DisplayName = "자기 자신"),
    Single UMETA(DisplayName = "단일"),
    Multi UMETA(DisplayName = "다수"),
    All UMETA(DisplayName = "모든 적/아군")
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Fenrir UMETA(DisplayName = "펜리르"),
    Surtr UMETA(DisplayName = "수르트"),
    Jormungandr UMETA(DisplayName = "요르문간드")
};


USTRUCT(BlueprintType)
struct FActionData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // --- 기본 정보 (General) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    FText DisplayName;

    // --- 타입 정보 (Type) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingType")
    ETargetingType TargetingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageType")
    EDamageType DamageType;

    // --- 비용 정보 (Cost) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
    int32 CostSP = 0;

    // --- 전투 수치 (Combat) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SkillCoefficient = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 NumberOfHits = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "1"))
    int32 NumberOfTargets = 1;

    // --- 상태 이상 (Status Effect) ---
    // 이 행동이 적용할 상태 이상의 ID (DT_StatusEffects의 Row Name)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
    FName StatusEffectIDToApply;

    // 상태 이상이 적용될 확률 (0.0 ~ 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StatusEffectChance = 1.0f;

    // 상태 이상의 지속 턴을 이 값으로 재정의합니다.
    // 0 이하의 값일 경우, DT_StatusEffects에 정의된 기본값을 사용합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect|Overrides")
    int32 StatusEffectDurationOverride = 0;

    // 상태 이상의 효과량(데미지, 스탯 감소량 등)을 이 값으로 재정의합니다.
    // 0.0일 경우, DT_StatusEffects에 정의된 기본값을 사용합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect|Overrides")
    float StatusEffectMagnitudeOverride = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ParryWindowDuration = 0.0f;

    // --- 로직 (Logic) ---
    // 이 행동의 실제 로직을 담고 있는 UGameAction 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
    TSubclassOf<UGameAction> GameActionClass;

};