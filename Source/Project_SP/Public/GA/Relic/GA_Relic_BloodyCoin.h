// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_BloodyCoin.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_BloodyCoin : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_Relic_BloodyCoin();
    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    UFUNCTION()
    void OnSkillKillReceived(FGameplayEventData Payload);

    // BP를 1 올려줄 Gameplay Effect (에디터에서 할당)
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TSubclassOf<class UGameplayEffect> AddBPGEClass;

};
