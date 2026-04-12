// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Tag/SPGameplayTags.h"
#include "Manager/SPGASBattleTypes.h"
#include "SPActiveEffectUIDictionary.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPActiveEffectUIDictionary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Dictionary")
	TMap<FGameplayTag, FEffectUIData> EffectUIDataMap;

	UFUNCTION(BlueprintCallable, Category = "UI Dictionary")
	FORCEINLINE bool GetEffectUIData(FGameplayTag Tag, FEffectUIData& OutData) const
	{
		if (const FEffectUIData* FoundData = EffectUIDataMap.Find(Tag))
		{
			OutData = *FoundData;
			return true; 
		}
		return false; 
	}
	
};
