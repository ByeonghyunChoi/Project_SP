// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "OpartsDefinition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Const)
class PROJECT_SP_API UOpartsDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 1. UI 표시용 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName; // 예: "크리스탈 오파츠"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// 2. GAS 능력 (패시브)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> PassiveAbilityClass;

	// 3. GAS 스탯 (기본 스탯 보너스)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> StatsEffectClass;

	// 4. 오파츠 식별 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTag OpartsTag;
};
