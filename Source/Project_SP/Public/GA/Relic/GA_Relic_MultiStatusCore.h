// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_MultiStatusCore.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_MultiStatusCore : public UGameplayAbility
{
	GENERATED_BODY()

public:
    UGA_Relic_MultiStatusCore();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    UFUNCTION()
    void OnHitEventReceived(FGameplayEventData Payload);

    // 상태이상 태그와 그에 맞는 GE를 연결할 사전(Map)
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TMap<FGameplayTag, TSubclassOf<class UGameplayEffect>> StatusEffectMap;
	
};
