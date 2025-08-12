// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArtifactTypes.generated.h" 

UENUM(BlueprintType)
enum class EArtifactType : uint8
{
	HPUp UMETA(DisplayName = "HP"),
	ATKUp UMETA(DisplayName = "ATK"),
	DEFUp UMETA(DisplayName = "DEF"),
	SPDUp UMETA(DisplayName = "SPD"),
	Special UMETA(DisplayName = "SP")
};

UENUM(BlueprintType)
enum class ERarity : uint8
{
	Normal UMETA(DisplayName = "NORMAL"),
	Rare UMETA(DisplayName = "RARE"),
	Unique UMETA(DisplayName = "UNIQUE"),
	Legendary UMETA(DisplayName = "LEGENDARY"),
	Mystic UMETA(DisplayName = "MYSTIC")
};

UENUM(BlueprintType)
enum class ESetType : uint8
{
	Crystal UMETA(DisplayName = "CRTSTAL"),
	Emerald UMETA(DisplayName = "EMERALD"),
	Gold UMETA(DisplayName = "GOLD")
};

UENUM(BlueprintType)
enum class ESpecialStatType : uint8
{
	Default UMETA(DisplayName = "기본"),
	CriticalChance UMETA(DisplayName = "치확"),
	CriticalDamage UMETA(DisplayName = "치피"),
	ArmorPenetration UMETA(DisplayName = "방무뎀"),
	AttackPercent UMETA(DisplayName = "공격력"),
	DefensePercent UMETA(DisplayName = "방어력"),
	HPPercent UMETA(DisplayName = "체력"),
	SpeedPercent UMETA(DisplayName = "속도")
};

UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
	MNormal UMETA(DisplayName = "일반몹"),
	MElite UMETA(DisplayName = "엘리트몹"),
	MBoss UMETA(DisplayName = "보스몹"),
};


USTRUCT(BlueprintType)
struct FStatBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESpecialStatType, float> SpecialStats;

	FStatBonus()
		: HP(0), Attack(0), Defense(0), Speed(0)
	{
	}
};