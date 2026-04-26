// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "Manager/SPGASBattleTypes.h"
#include "WeaponAbilityData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UWeaponAbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 이 데이터가 어떤 무기인지 (예: Weapon.Fenrir)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponTag;

	// 무기 실제 외형
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TObjectPtr<class UStaticMesh> WeaponMesh;

	// 1. 일반 공격 GA 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> NormalAttackAbility;

	// 2. 무기 스킬 GA 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> WeaponSkillAbility;

	// 3. 패링 스킬 GA 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> ParrySkillAbility;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	ETargetingType NormalAttackTargeting = ETargetingType::Single;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	ETargetingType WeaponSkillTargeting = ETargetingType::Single;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	ETargetingType ParrySkillTargeting = ETargetingType::Single;

};
