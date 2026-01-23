// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SPGASMonsterState.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"

ASPGASMonsterState::ASPGASMonsterState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("GAS"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<USPGASAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ASPGASMonsterState::GetAbilitySystemComponent() const
{
	return ASC;
}
