// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeSet/SPGASAttributeSet.h"
#include "GameplayEffectExtension.h"

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

	// 1. 최대 체력이 변경될 때 (오파츠 장착, 레벨업 등)
	if (Attribute == GetMaxHealthAttribute())
	{
		// 기존의 MaxHealth 값을 가져옵니다.
		float OldMaxHealth = GetMaxHealth();

		// 0으로 나누기 방지 (초기화 단계일 수 있음)
		if (OldMaxHealth > 0.0f)
		{
			// 고정 수치만큼 증가 (최대 체력이 100 늘어나면 현재 체력도 +100) -> 기획 의도에 더 맞음
			float Delta = NewValue - OldMaxHealth; // 늘어난 양

			// 현재 체력에 늘어난 양만큼 더해줍니다.
			float NewHealth = GetHealth() + Delta;
			SetHealth(NewHealth);
		}
	}
	// 2. 체력 보정 (클램핑)
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth()); // 0 ~ MaxHealth 사이로 제한
	}
}

void USPGASAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}
