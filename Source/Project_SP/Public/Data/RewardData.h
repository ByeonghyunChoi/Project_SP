// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RewardData.generated.h"

USTRUCT(BlueprintType)
struct FRewardData : public FTableRowBase
{
	GENERATED_BODY()

	
	FRewardData()
		: RewardName(FText::GetEmpty())
		, SandAmount(0)
		, IncompleteEnergyAmount(0)
		, MoneyAmount(0)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Reward")
	FText RewardName; // 보상 이름
	UPROPERTY(EditAnywhere, Category = "Reward")
	int32 SandAmount; // 모래 수량
	UPROPERTY(EditAnywhere, Category = "Reward")
	int32 IncompleteEnergyAmount; // 불완전한 기운 수량
	UPROPERTY(EditAnywhere, Category = "Reward")
	int32 MoneyAmount; // 돈 수량

};