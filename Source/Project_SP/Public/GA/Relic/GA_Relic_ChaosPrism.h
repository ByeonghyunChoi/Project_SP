// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_Relic_ChaosPrism.generated.h"

UCLASS()
class PROJECT_SP_API UGA_Relic_ChaosPrism : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_Relic_ChaosPrism();
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
    UFUNCTION()
    void OnKillEventReceived(FGameplayEventData Payload);

    // 어떤 상태이상을 전이할 수 있는지 등록해두는 사전 (에디터에서 세팅)
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TMap<FGameplayTag, TSubclassOf<class UGameplayEffect>> StatusEffectMap;

};
