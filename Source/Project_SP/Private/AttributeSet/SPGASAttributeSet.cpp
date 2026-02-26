// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeSet/SPGASAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/SPGASPlayerCharacter.h"

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

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		SetIncomingDamage(0.0f); // 초기화

		if (Damage > 0.0f)
		{
			float CurrentHealth = GetHealth();
			SetHealth(FMath::Clamp(CurrentHealth - Damage, 0.0f, GetMaxHealth()));

			UE_LOG(LogTemp, Warning, TEXT("데미지 적용됨! -%f, 남은 체력: %f"), Damage, GetHealth());

			// 🌟 [핵심] 체력을 깎은 바로 이 시점에! 죽었는지 살았는지 확인해야 합니다!
			if (GetHealth() <= 0.0f)
			{
				ASPGASPlayerCharacter* PlayerCharacter = Cast<ASPGASPlayerCharacter>(GetOwningActor());
				if (PlayerCharacter)
				{
					if (GetTimePower() >= 20.0f)
					{
						// 1) 시간의 힘 20 삭감
						SetTimePower(GetTimePower() - 20.0f);

						// 2) 최대 체력의 40%로 부활!
						float ReviveHealth = GetMaxHealth() * 0.4f;
						SetHealth(ReviveHealth);

						UE_LOG(LogTemp, Warning, TEXT("시간의 힘 20을 소모하여 체력 %.0f(40%%)로 부활합니다! 남은 시간의 힘: %.0f"), ReviveHealth, GetTimePower());
					}
					else
					{
						// 시간의 힘 부족 (진짜 게임 오버)
						UE_LOG(LogTemp, Error, TEXT("시간의 힘이 부족하여 사망했습니다."));
						// TODO: 사망 처리 로직 호출
					}
				}
			}
		}
	}

	// 2. 시간의 힘(PowerOfTime)이 깎인 상황인지 확인
	if (Data.EvaluatedData.Attribute == GetTimePowerAttribute())
	{
		if (GetTimePower() <= 0.0f)
		{
			SetTimePower(0.0f);
			UE_LOG(LogTemp, Error, TEXT("시간의 힘이 모두 고갈되었습니다! 로비로 귀환합니다."));
			// TODO: 강제 로비 귀환 로직 호출
		}
	}
}
