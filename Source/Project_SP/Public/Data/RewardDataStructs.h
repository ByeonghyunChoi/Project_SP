// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/InventoryComponent.h"
#include "RewardDataStructs.generated.h"

USTRUCT(BlueprintType)
struct FCombatRewardInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EResourceType ResourceType; // 아까 만든 Enum (Energy, Sand, Fragment, Gold)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinAmount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmount = 20;
};

USTRUCT(BlueprintType)
struct FCombatRewardRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 한 전투가 끝날 때 '무조건' 주는 재화들의 리스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCombatRewardInfo> GuaranteedRewards;
};