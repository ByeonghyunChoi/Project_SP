// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/ActionData.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECT_SP_API UWeapon : public UObject
{
	GENERATED_BODY()

public:
    // 무기의 속성 (Fenrir, Surtr, Jormungandr)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    EDamageType WeaponType;

    // 이 무기를 장착했을 때 사용할 평타 스킬의 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    FName BasicAttackActionID;

    // 이 무기를 장착했을 때 사용할 메인 스킬의 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    FName MainSkillActionID;

    // 이 무기로 패링(스위치) 성공 시 발동할 스킬의 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    FName SwitchSkillActionID;
	
};
