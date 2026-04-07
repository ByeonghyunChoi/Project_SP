// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Tag/SPGameplayTags.h"
#include "ScalableFloat.h"
#include "SPMonsterData.generated.h"

UENUM(BlueprintType)
enum class EMonsterRank : uint8
{
	Normal  UMETA(DisplayName = "일반"),
	Epic    UMETA(DisplayName = "에픽"),
	Boss    UMETA(DisplayName = "보스")
};

USTRUCT(BlueprintType)
struct FMonsterBaseStats
{
	GENERATED_BODY()

	// 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Level = 1.f;
	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalableFloat MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalableFloat Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalableFloat Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalableFloat Speed;
};

UCLASS()
class PROJECT_SP_API USPMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//몬스터 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Info")
	FText MonsterName;
	
	//몬스터 등급
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Info")
	EMonsterRank MonsterRank = EMonsterRank::Normal;

	//몬스터 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Info")
	TSoftObjectPtr<UTexture2D> MonsterIcon;

	//해당 몬스터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Info")
	TSubclassOf<class ASPGASMonsterCharacter> MonsterClass;

	// 몬스터 스텟
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Combat")
	FMonsterBaseStats BaseStats;

	// 몬스터 약점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Combat")
	FGameplayTagContainer WeaknessTags;

	/// <몬스터가 플레이어에게 줄 기본 보상>

	// 경험지 보상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3. Reward")
	int32 ExpReward = 50;
	// 돈 보상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3. Reward")
	int32 MoneyReward = 100;

	/// <기획에 따라 추후 추가>

	// 고유 ID 설정
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("MonsterData", GetFName());
	}
};
