// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArtifactTypes.h"
#include "OrpartsData.generated.h"

USTRUCT(BlueprintType)
struct FOrpartsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESetType SetType; //타입
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;//오파츠 레벨

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStatBonus BaseStats; //기본 스탯

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESpecialStatType, float> SpecialStats; //특수 스탯

	FOrpartsData()
		: Level(1)
	{

	}
};
