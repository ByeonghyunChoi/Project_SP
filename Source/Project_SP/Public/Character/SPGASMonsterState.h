// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "SPGASMonsterState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API ASPGASMonsterState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASPGASMonsterState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<class USPGASAttributeSet> AttributeSet;
	
public:
	FORCEINLINE class USPGASAttributeSet* GetAttributeSet() const { return AttributeSet; }
};
