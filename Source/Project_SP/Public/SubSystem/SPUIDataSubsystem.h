// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tag/SPGameplayTags.h"
#include "Manager/SPGASBattleTypes.h"
#include "SPUIDataSubsystem.generated.h"

class USPActiveEffectUIDictionary;

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPUIDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SP | UI Dictionary")
	bool GetEffectUIData(FGameplayTag EffectTag, FEffectUIData& OutData);

private:
	UPROPERTY()
	TObjectPtr<USPActiveEffectUIDictionary> GlobalUIDictionary;
};
