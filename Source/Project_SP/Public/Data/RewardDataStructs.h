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
	EResourceType ResourceType; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinAmount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmount = 20;

	FCombatRewardInfo()
	{
		ResourceType = EResourceType::Gold;
	}
};

USTRUCT(BlueprintType)
struct FRewardResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Gold = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Sand = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 IncompleteEnergy = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Fragment = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Exp = 0;

	// 유물을 몇 번 뽑을 수 있는지 (강적 맵에서 최대 2가 될 수 있음)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RelicRewardCount = 0;

	// 보스 보상인가? (60% 에픽, 40% 유니크 판별용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsBossReward = false;

	// 구조체 더하기 연산자 오버로딩 (보상 누적을 편하게 하기 위함)
	FRewardResult& operator+=(const FRewardResult& Other)
	{
		Gold += Other.Gold;
		Sand += Other.Sand;
		IncompleteEnergy += Other.IncompleteEnergy;
		Fragment += Other.Fragment;
		Exp += Other.Exp;
		RelicRewardCount += Other.RelicRewardCount;
		// bIsBossReward는 직접 제어하므로 합치지 않음
		return *this;
	}
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