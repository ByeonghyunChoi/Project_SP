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

USTRUCT(BlueprintType)
struct FActionUIData
{
	GENERATED_BODY()

	// 1. 스킬 이름 (예: "염룡의 참격")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ActionName;

	// 2. 스킬 종류 (예: "단일 공격", "광역 스킬", "반격기")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ActionCategory;

	// 3. 스킬 내용 (MultiLine으로 에디터에서 줄바꿈 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MultiLine = true))
	FText ActionDescription;

	// 4. 무기/스킬 아이콘 (UI의 Image 위젯에 넣을 텍스처)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UTexture2D> ActionIcon;
};

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
	TObjectPtr<class USkeletalMesh> WeaponMesh;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Info")
	FText NormalAttackUIText; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Info")
	FText WeaponSkillUIText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Info")
	FActionUIData NormalAttackUIData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Info")
	FActionUIData WeaponSkillUIData;

};
