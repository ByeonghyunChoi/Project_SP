// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"

ASPGASPlayerState::ASPGASPlayerState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("GAS"));
	AttributeSet = CreateDefaultSubobject<USPGASAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ASPGASPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

