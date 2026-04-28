// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeSet/SPGASAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASCharacterBase.h"

USPGASAttributeSet::USPGASAttributeSet()  
{
	InitHealth(1.0f);
	InitMaxHealth(1.0f);
	InitLevel(1.0f);
	InitSpeed(1.0f);
	InitMaxActionGauge(100.0f); 
	InitMaxExperience(3000.0f);  

	InitAttack(0.0f);
	InitDefense(0.0f);
	InitBattlePoint(0.0f);
	InitMaxBattlePoint(5.0f);
	InitTimePower(0.0f);
	InitMaxTimePower(200.0f);
	InitActionGauge(0.0f);
	InitExperience(0.0f);

	InitDefenseIgnore(0.0f);
	InitCriticalRate(0.0f);
	InitCriticalDamage(0.0f);
	InitEffectHitRate(0.0f);
	InitEffectAmplify(0.0f);
	InitOutgoingDamageMultiplier(0.0f);
	InitIncomingDamageMultiplier(0.0f);

	InitIncomingDamage(0.0f);
	InitIncomingHeal(0.0f);
	InitIncomingIsCritical(0.0f);
}


void USPGASAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetBattlePointAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxBattlePoint());
	}
	else if (Attribute == GetTimePowerAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxTimePower());
	}
	else if (Attribute == GetActionGaugeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxActionGauge());
	}
	else if (Attribute == GetAttackAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
	else if (Attribute == GetCriticalRateAttribute())
	{
		// 1. 내 몸에 골드버그 태그가 있다면 어떠한 변화가 와도 무조건 0으로 덮어씌움
		if (GetOwningAbilitySystemComponent()->HasMatchingGameplayTag(FSPGameplayTags::Get().Oparts_GoldBug))
		{
			NewValue = 0.0f;
		}
		else
		{
			// 2. 골드버그가 없다면 정상적으로 마이너스가 되지 않게만(0 이상) 보정
			NewValue = FMath::Max(NewValue, 0.0f);
		}
	}
	else if (Attribute == GetCriticalDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetOutgoingDamageMultiplierAttribute() ||
		Attribute == GetIncomingDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void USPGASAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		bool bIsCritical = GetIncomingIsCritical() > 0.0f;
		SetIncomingDamage(0.0f); 
		SetIncomingIsCritical(0.0f);

		if (Damage > 0.0f)
		{
			float CurrentHealth = GetHealth();
			SetHealth(FMath::Clamp(CurrentHealth - Damage, 0.0f, GetMaxHealth()));

			OnDamageTakenEvent.Broadcast(Damage, bIsCritical);
		}
	}

	else if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		float Heal = GetIncomingHeal();
		SetIncomingHeal(0.0f); 

		if (Heal > 0.0f)
		{
			float CurrentHealth = GetHealth();
			SetHealth(FMath::Clamp(CurrentHealth + Heal, 0.0f, GetMaxHealth()));
		}
	}

	else if (Data.EvaluatedData.Attribute == GetTimePowerAttribute())
	{
		float CurrentPower = GetTimePower();
		SetTimePower(FMath::Clamp(CurrentPower, 0.0f, GetMaxTimePower()));
	}
}
