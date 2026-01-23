// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_Interact.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_Interact : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	USPGA_Interact();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractionRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool bShowDebug = true;
};
