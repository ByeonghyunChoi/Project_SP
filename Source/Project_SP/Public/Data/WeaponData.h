// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ActionData.h"
#include "WeaponData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECT_SP_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

    UWeaponData()
        : WeaponType(EDamageType::Fenrir) // Enum의 첫 번째 값
        , BasicAttackActionID(NAME_None)
        , SpecialSkillActionID(NAME_None)
        , ParrySkillActionID(NAME_None)
        , DisplayName(FText::GetEmpty())
        , Description(FText::GetEmpty())
    {
    }

    // --- 로직용 데이터 ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
    EDamageType WeaponType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
    FName BasicAttackActionID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
    FName SpecialSkillActionID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
    FName ParrySkillActionID;

    // --- UI용 데이터 ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    FText Description;
	
};
