// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Tag/SPGameplayTags.h"
#include "SPGASAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API ASPGASAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASPGASAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	void OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount);

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBehaviorTree> FieldBT;

private:
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> CachedASC;

	
};
