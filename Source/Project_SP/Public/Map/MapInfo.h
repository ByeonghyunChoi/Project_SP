#pragma once

#include "CoreMinimal.h"
#include "MapInfo.generated.h"

UENUM(BlueprintType)
enum class EMapState :uint8
{
    InProgress UMETA(DisplayName = "맵 진행 중"),
    Reward UMETA(DisplayName = "보상 수령 중"),
    Cleared UMETA(DisplatName = "맵 클리어")
};

UENUM(BlueprintType)
enum class EMapType :uint8
{
    // "일반" 등급 맵 유형
    Rest            UMETA(DisplayName = "쉼터"),
    NormalBattle    UMETA(DisplayName = "일반 전투"),
    // "에픽" 등급 맵 유형
    StrongEnemyBattle UMETA(DisplayName = "강적 전투"),
    Jester            UMETA(DisplayName = "광대 맵"),
    // "준비" 등급 맵 유형
    Prepare           UMETA(DisplayName = "준비 맵"),
    // "보스" 등급 맵 유형
    BossBattle        UMETA(DisplayName = "보스 전투")
};

UENUM(BlueprintType)
enum class EMapGrade : uint8
{
    Normal UMETA(DisplayName = "일반 등급 맵"),
    Epic UMETA(DisplayName = "에픽 등급 맵"),
    Prepare UMETA(DisplayName = "준비 등급 맵"),
    Boss UMETA(DisplayName = "보스 등급 맵")
};