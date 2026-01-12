// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASPlayerState.h"
#include "AbilitySystemComponent.h"

ASPGASPlayerState::ASPGASPlayerState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("GAS"));
}

UAbilitySystemComponent* ASPGASPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

