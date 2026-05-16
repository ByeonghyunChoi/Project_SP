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

UENUM(BlueprintType)
enum class EMonsterAICondition : uint8
{
	Always			UMETA(DisplayName = "항상"),
	TurnCount		UMETA(DisplayName = "특정 턴 마다"),
	HP_Below		UMETA(DisplayName = "체력이 특정 % 이하일 때"),
	WasAttacked		UMETA(DisplayName = "이전 턴에 피격 당했을 때"),
	AllyCount_Below	UMETA(DisplayName = "생존한 아군 수가 특정 수 이하일 때"),
	HasGameplayTag	UMETA(DisplayName = "특정 상태 태그가 있을 때")
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

USTRUCT(BlueprintType)
struct FMonsterSkillUIInfo
{
	GENERATED_BODY()

	// 스킬 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill UI")
	FText SkillName;

	// 스킬 종류 (텍스트로 바로 입력. 예: "일반 공격", "스킬", "필살기")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill UI")
	FText SkillType;

	// 스킬 내용 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill UI", meta = (MultiLine = true))
	FText SkillDescription;
};

USTRUCT(BlueprintType)
struct FMonsterAIPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMonsterAICondition Condition = EMonsterAICondition::Always;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Condition != EMonsterAICondition::Always && Condition != EMonsterAICondition::WasAttacked && Condition != EMonsterAICondition::HasGameplayTag"))
	float ConditionValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Condition == EMonsterAICondition::HasGameplayTag"))
	FGameplayTag RequiredTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityToExecute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
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

	// 몬스터가 보유한 스킬 리스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Combat")
	TArray<FMonsterSkillUIInfo> SkillList;

	//몬스터의 AI 패턴 리스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Combat | AI Pattern")
	TArray<FMonsterAIPattern> AIPatterns;

	// 고유 ID 설정
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("MonsterData", GetFName());
	}
};
