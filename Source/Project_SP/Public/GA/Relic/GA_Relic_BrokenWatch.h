// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_BrokenWatch.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_BrokenWatch : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_Relic_BrokenWatch();
    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    UFUNCTION()
    void OnParrySuccessReceived(FGameplayEventData Payload);

    // 체력을 회복시켜줄 Gameplay Effect (에디터에서 할당)
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TSubclassOf<class UGameplayEffect> HealGEClass;
};
