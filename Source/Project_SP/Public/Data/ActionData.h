#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Combat/GameAction.h" 
#include "ActionData.generated.h"


/**
 *
 */
UENUM(BlueprintType)
enum class EActionType : uint8
{
    Attack UMETA(DisplayName = "공격"),
    Skill UMETA(DisplayName = "스킬"),
    Parry UMETA(DisplayName = "패링"),
    Wait UMETA(DisplayName = "대기")
};

UENUM(BlueprintType)
enum class ETargetingType : uint8
{
    Single UMETA(DisplayName = "단일"),
    Dual UMETA(DisplayName = "2인"),
    All UMETA(DisplayName = "모든 적")
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Fenrir UMETA(DisplayName = "펜리르"),
    Surtr UMETA(DisplayName = "수르트"),
    Jormungandr UMETA(DisplayName = "요르문간드")
};

UENUM(BlueprintType)
enum class ECostType : uint8
{
    None UMETA(DisplayName = "없음"),
    SP UMETA(DisplayName = "스위치 포인트")
};

USTRUCT(BlueprintType)
struct FActionData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 행동의 고유 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    FName ActionID;
    // 행동 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
    FText DisplayName;

    //행동 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EActionType ActionType;

    //타겟팅 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    ETargetingType TargetingType;

    //사용 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
    ECostType CostType;

    //자원 사용량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
    float CostAmount = 0.0f;

    //스킬 계수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
    float SkillCoefficient;

    // 이 행동이 적용할 상태 이상의 ID (DT_StatusEffects의 Row Name)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FName StatusEffectIDToApply;

    // 상태 이상이 적용될 확률 (0.0 ~ 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float StatusEffectChance;

    //공격 횟수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    int32 NumberOfHits = 1;

    //데미지 타입(속성)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EDamageType DamageType;

    //행동을 실행할 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Class")
    TSubclassOf<UGameAction> GameActionClass;

};