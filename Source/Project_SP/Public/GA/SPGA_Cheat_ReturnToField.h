// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_Cheat_ReturnToField.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_Cheat_ReturnToField : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	USPGA_Cheat_ReturnToField();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
