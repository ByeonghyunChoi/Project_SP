// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArtifactTypes.generated.h" 

UENUM(BlueprintType)
enum class EArtifactType : uint8
{
	HPUp,
	ATKUp,
	DEFUp,
	SPDUp,
	Special
};

UENUM(BlueprintType)
enum class ERarity : uint8
{
	Normal,
	Rare,
	Unique,
	Legendary,
	Mystic
};

UENUM(BlueprintType)
enum class ESetType : uint8
{
	Crystal,
	Emerald,
	Gold
};

UENUM(BlueprintType)
enum class ESpecialStatType : uint8
{
	CriticalChance,
	CriticalDamage,
	ArmorPenetration,
	AttackPercent,
	DefensePercent,
	HPPercent,
	SpeedPercent
};

UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
	MNormal,
	MElite,
	MBoss,
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