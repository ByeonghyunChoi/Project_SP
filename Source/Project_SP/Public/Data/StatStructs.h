// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StatStructs.generated.h"

USTRUCT(BlueprintType)
struct FOpartsStatRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Attack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.f;
};

USTRUCT(BlueprintType)
struct FStatModifiers
{
	GENERATED_BODY()

public:
	// 1. 고정 수치 (Flat): 오파츠 레벨업 등으로 오르는 깡스탯 (예: +100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Flat")
	float FlatHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Flat")
	float FlatAttack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Flat")
	float FlatSpeed = 0.f;

	// 2. 비율 수치 (Percent): 아티팩트/유물 효과 (예: 20% -> 0.2f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Percent")
	float PercentHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Percent")
	float PercentAttack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Percent")
	float PercentSpeed = 0.f;

	// 3. 기타 스탯 (합연산)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Other")
	float CriticalChance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Other")
	float CriticalDamage = 0.f;

	// 생성자 (초기화)
	FStatModifiers() {}

	// + 연산자 오버로딩 (구조체끼리 편하게 더하기 위해)
	FStatModifiers operator+(const FStatModifiers& Other) const
	{
		FStatModifiers Result;

		// 고정 수치 합산
		Result.FlatHP = FlatHP + Other.FlatHP;
		Result.FlatAttack = FlatAttack + Other.FlatAttack;
		Result.FlatSpeed = FlatSpeed + Other.FlatSpeed;

		// 비율 수치 합산
		Result.PercentHP = PercentHP + Other.PercentHP;
		Result.PercentAttack = PercentAttack + Other.PercentAttack;
		Result.PercentSpeed = PercentSpeed + Other.PercentSpeed;

		// 기타 스탯 합산
		Result.CriticalChance = CriticalChance + Other.CriticalChance;
		Result.CriticalDamage = CriticalDamage + Other.CriticalDamage;

		return Result;
	}
};