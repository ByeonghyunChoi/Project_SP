// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_CombatActionBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECT_SP_API USPGA_CombatActionBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USPGA_CombatActionBase();

public:
	UFUNCTION(BlueprintCallable, Category = "Battle Action")
	void CompleteBattleAction();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Battle Action")
	void ExecuteBattleAction();

	// 행동이 시작되기 전에 필요한 데이터 준비
	virtual void PrepareBattleAction(const FGameplayEventData* TriggerEventData);

	virtual bool CommitBattleAction(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	virtual void SetupActionEventListeners();

	virtual bool ValidateBattleAction() const;

private:
	bool bBattleActionCompleted = false;

};
