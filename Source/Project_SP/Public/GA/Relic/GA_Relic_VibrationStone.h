// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_VibrationStone.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_VibrationStone : public UGameplayAbility
{
	GENERATED_BODY()

public:
    UGA_Relic_VibrationStone();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    UFUNCTION()
    void OnBattleStartReceived(FGameplayEventData Payload);

    // 에디터에서 넣어줄 'BP +1 회복' GE
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TSubclassOf<class UGameplayEffect> AddBPGEClass;
};