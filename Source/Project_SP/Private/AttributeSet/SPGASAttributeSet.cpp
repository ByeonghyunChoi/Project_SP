// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeSet/SPGASAttributeSet.h"

USPGASAttributeSet::USPGASAttributeSet() :
	Health(100.f)
	, MaxHealth(100.f)
	, AttackPower(10.f)
	, Speed(100.f)          
	, ActionGauge(0.f)      
	, CritChance(0.05f)     
	, CritDamage(0.5f)      
	, DamageDealtInc(0.f)   
	, DamageDealtDec(0.f)
	, DamageReceivedInc(0.f)
	, DamageReceivedDec(0.f)
	, Durability(0.f)
	, IgnoreDurability(0.f)
	, IgnoreLevelScaling(0.f)
	, EffectEfficiency(0.f) 
	, EffectProbability(0.f) 
	, Level(1.f)
	, Experience(0.f)
	, TimePower(100.f)      
{
}

void USPGASAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetActionGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 100.f);
	}
	else if (Attribute == GetCritChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.0f);
	}
}

void USPGASAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}
