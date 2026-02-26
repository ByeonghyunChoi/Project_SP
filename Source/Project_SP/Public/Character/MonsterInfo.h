#pragma once

#include "CoreMinimal.h"
#include "MonsterInfo.generated.h"

UENUM(BlueprintType)
enum class EEnemyRank :uint8
{
    Normal,
    Epic,
    Boss
};