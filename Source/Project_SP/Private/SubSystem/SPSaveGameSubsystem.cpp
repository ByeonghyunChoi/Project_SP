// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPSaveGameSubsystem.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

void USPSaveGameSubsystem::SavePlayerStats(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn);
	if (!ASI) return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	// [저장 로직]
	// 1. 자원 (Health, TimePower) -> 현재 값(Current Value) 저장
	//    이유: 맞아서 깎인 체력 그대로 다음 맵으로 가야 하니까.
	bool bFound = false;
	SaveData.Stats.CurrentHealth = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetHealthAttribute(), bFound);
	SaveData.Stats.CurrentTimePower = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetTimePowerAttribute(), bFound);

	// 2. 능력치 (MaxHealth, Attack...) -> 기본 값(Base Value) 저장
	//    이유: '공격력 증가 버프' 같은 일시적인 효과는 저장하면 안 되니까.
	//    (만약 아이템으로 영구 증가한 스탯이 Base를 올려주는 방식이라면 이게 맞음)

	SaveData.Stats.MaxHealth = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute());
	SaveData.Stats.AttackPower = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetAttackPowerAttribute());
	SaveData.Stats.Speed = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute());

	SaveData.Stats.CritChance = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCritChanceAttribute());
	SaveData.Stats.CritDamage = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCritDamageAttribute());

	SaveData.Stats.DamageDealtInc = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDamageDealtIncAttribute());
	SaveData.Stats.DamageDealtDec = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDamageDealtDecAttribute());
	SaveData.Stats.DamageReceivedInc = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDamageReceivedIncAttribute());
	SaveData.Stats.DamageReceivedDec = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDamageReceivedDecAttribute());

	SaveData.Stats.Durability = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDurabilityAttribute());
	SaveData.Stats.IgnoreDurability = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetIgnoreDurabilityAttribute());
	SaveData.Stats.EffectEfficiency = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectEfficiencyAttribute());
	SaveData.Stats.EffectProbability = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectProbabilityAttribute());

	SaveData.Stats.Level = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute());
	SaveData.Stats.Experience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute());

	UE_LOG(LogTemp, Log, TEXT("💾 [SaveSystem] Stats Saved. HP: %.1f / %.1f"), SaveData.Stats.CurrentHealth, SaveData.Stats.MaxHealth);
}

void USPSaveGameSubsystem::LoadPlayerStats(APawn* PlayerPawn)
{
	if (!SaveData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ [SaveSystem] No Valid Data to Load."));
		return;
	}

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn);
	if (!ASI) return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	// [불러오기 로직]
	// 저장된 값으로 캐릭터의 '기본 스탯'을 덮어씌움

	// 1. 순서 중요: MaxHealth를 먼저 세팅해야 Health 클램핑에 문제가 없음
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute(), SaveData.Stats.MaxHealth);

	// 2. 나머지 스탯들 복구
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetAttackPowerAttribute(), SaveData.Stats.AttackPower);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute(), SaveData.Stats.Speed);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCritChanceAttribute(), SaveData.Stats.CritChance);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCritDamageAttribute(), SaveData.Stats.CritDamage);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDamageDealtIncAttribute(), SaveData.Stats.DamageDealtInc);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDamageDealtDecAttribute(), SaveData.Stats.DamageDealtDec);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDamageReceivedIncAttribute(), SaveData.Stats.DamageReceivedInc);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDamageReceivedDecAttribute(), SaveData.Stats.DamageReceivedDec);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDurabilityAttribute(), SaveData.Stats.Durability);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetIgnoreDurabilityAttribute(), SaveData.Stats.IgnoreDurability);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetEffectEfficiencyAttribute(), SaveData.Stats.EffectEfficiency);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetEffectProbabilityAttribute(), SaveData.Stats.EffectProbability);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute(), SaveData.Stats.Level);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), SaveData.Stats.Experience);

	// 3. 자원 복구 (마지막에)
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), SaveData.Stats.CurrentHealth);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), SaveData.Stats.CurrentTimePower);

	UE_LOG(LogTemp, Log, TEXT("♻️ [SaveSystem] Stats Loaded. HP: %.1f"), SaveData.Stats.CurrentHealth);
}

void USPSaveGameSubsystem::ResetSaveData()
{
	SaveData.Reset();
	UE_LOG(LogTemp, Log, TEXT("🧹 [SaveSystem] Data Reset."));
}