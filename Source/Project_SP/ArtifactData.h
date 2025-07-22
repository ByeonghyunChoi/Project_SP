// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArtifactTypes.h"
#include "ArtifactData.generated.h"

USTRUCT(BlueprintType)
struct FArtifactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArtifactType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESetType SetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FStatBonus StatBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESpecialStatType SpecialStat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpecialValue;

    FArtifactData()
        : ID(NAME_None), SpecialValue(0.f)
    {
    }
};
