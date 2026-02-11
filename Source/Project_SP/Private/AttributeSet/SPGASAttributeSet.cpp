// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeSet/SPGASAttributeSet.h"
#include "GameplayEffectExtension.h"

USPGASAttributeSet::USPGASAttributeSet()  
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitBattlePoint(0.0f);
	InitMaxBattlePoint(5.0f);
	InitTimePower(200.0f);
	InitMaxTimePower(9999.0f);
	InitDefense(10.0f);
	InitSpeed(100.0f);
	InitActionGauge(0.0f);
	InitMaxActionGauge(100.0f);
	InitAttack(20.0f);
	InitDefenseIgnore(0.0f);
	InitCriticalRate(0.05f);
	InitCriticalDamage(1.5f);
	InitEffectHitRate(0.0f);
	InitEffectAmplify(0.0f);
	InitOutgoingDamageMultiplier(1.0f);
	InitIncomingDamageMultiplier(1.0f);
	InitLevel(1.0f);
	InitExperience(0.0f);
	InitMaxExperience(100.0f);
	InitResistanceSurtr(0.0f);
	InitResistanceFenrir(0.0f);
	InitResistanceJormungandr(0.0f);
	InitIncomingDamage(0.0f);
	InitIncomingHeal(0.0f);
}

void USPGASAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		// 체력은 0 ~ MaxHealth 사이여야 함
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
		// 행동 게이지는 턴 매니저 규칙에 따라 0 ~ Max
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxActionGauge());
	}
	else if (Attribute == GetAttackAttribute())
	{
		// 공격력은 최소 0.0f
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		// 방어력도 최소 0.0f
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetSpeedAttribute())
	{
		// 속도가 음수면 턴 계산이 고장나므로 최소 0.0f (혹은 최소 1.0f)
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetCriticalRateAttribute())
	{
		// 치명타 확률은 0% 이상이어야 함
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetCriticalDamageAttribute())
	{
		// 치명타 피해는 기본 1.5
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

	// 메타 속성 데미지(데미지나 힐 로직 판정)
	// 게임 플레이 이벤트 전송(UI 및 애니메이션 연동, 데미지 숫자 등)
	// 상태 태그 관리(사망 태그 관리)
}
