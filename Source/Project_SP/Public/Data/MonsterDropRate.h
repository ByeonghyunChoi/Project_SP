#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterDropRate.generated.h"

// 몬스터 등급을 정의하는 열거형입니다.
UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
    EMG_Normal UMETA(DisplayName = "Normal"),
    EMG_Epic UMETA(DisplayName = "Epic"),
    EMG_Boss UMETA(DisplayName = "Boss")
};

// 몬스터 등급별 드랍 확률을 담는 구조체입니다.
USTRUCT(BlueprintType)
struct FMonsterDropRate : public FTableRowBase
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Grade")
	EMonsterGrade MonsterGrade;

    // 몬스터 등급에 따른 드랍 확률
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Rate")
    float NoDropChance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Rate")
    float NormalChance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Rate")
    float RareChance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Rate")
    float UniqueChance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Rate")
    float LegendaryChance;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Rate")
    float MysticChance;
};