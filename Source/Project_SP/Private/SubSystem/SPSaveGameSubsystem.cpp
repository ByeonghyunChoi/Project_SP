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
	// 1. 자원 (Current Value)
	SaveData.Stats.CurrentHealth = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetHealthAttribute(), bFound);
	SaveData.Stats.CurrentBattlePoint = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute());
	SaveData.Stats.CurrentTimePower = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetTimePowerAttribute(), bFound);
	SaveData.Stats.CurrentActionGauge = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute());

	// 2. 최대치 (Base Value) - 아이템으로 늘어난 Max는 아이템 로직이 처리
	SaveData.Stats.MaxHealth = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute());
	SaveData.Stats.MaxBattlePoint = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxBattlePointAttribute());
	SaveData.Stats.MaxTimePower = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxTimePowerAttribute());

	// 3. 기본 스탯 (Base Value)
	SaveData.Stats.Attack = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute());
	SaveData.Stats.Defense = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute());
	SaveData.Stats.Speed = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute());

	// 4. 전투 보조 (Base Value)
	SaveData.Stats.DefenseIgnore = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDefenseIgnoreAttribute());
	SaveData.Stats.CriticalRate = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCriticalRateAttribute());
	SaveData.Stats.CriticalDamage = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCriticalDamageAttribute());
	SaveData.Stats.EffectHitRate = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectHitRateAttribute());
	SaveData.Stats.EffectAmplify = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectAmplifyAttribute());

	// 5. 배율 (Base Value)
	SaveData.Stats.OutgoingDamageMultiplier = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute());
	SaveData.Stats.IncomingDamageMultiplier = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetIncomingDamageMultiplierAttribute());

	// 6. 성장 (Base Value)
	SaveData.Stats.Level = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute());
	SaveData.Stats.Experience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute());
	SaveData.Stats.MaxExperience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxExperienceAttribute());

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] Stats Saved. HP: %.1f / %.1f, Level: %.0f"),
		SaveData.Stats.CurrentHealth, SaveData.Stats.MaxHealth, SaveData.Stats.Level);
}

void USPSaveGameSubsystem::LoadPlayerStats(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	if (!SaveData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSystem] No Valid Data to Load."));
		return;
	}

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn);
	if (!ASI) return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	// 1. 최대치 및 성장 스탯 먼저 복구 (SetBase)
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute(), SaveData.Stats.MaxHealth);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxBattlePointAttribute(), SaveData.Stats.MaxBattlePoint);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxTimePowerAttribute(), SaveData.Stats.MaxTimePower);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute(), SaveData.Stats.Level);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), SaveData.Stats.Experience);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetMaxExperienceAttribute(), SaveData.Stats.MaxExperience);

	// 2. 기본 스탯 복구
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute(), SaveData.Stats.Attack);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute(), SaveData.Stats.Defense);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute(), SaveData.Stats.Speed);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetDefenseIgnoreAttribute(), SaveData.Stats.DefenseIgnore);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCriticalRateAttribute(), SaveData.Stats.CriticalRate);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetCriticalDamageAttribute(), SaveData.Stats.CriticalDamage);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetEffectHitRateAttribute(), SaveData.Stats.EffectHitRate);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetEffectAmplifyAttribute(), SaveData.Stats.EffectAmplify);

	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute(), SaveData.Stats.OutgoingDamageMultiplier);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetIncomingDamageMultiplierAttribute(), SaveData.Stats.IncomingDamageMultiplier);

	// 3. 자원 (Current Value) 복구 - 가장 마지막에!
	// (AttributeSet의 PreAttributeChange에서 Clamp가 작동하므로, Max가 이미 설정되어 있어야 함)
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), SaveData.Stats.CurrentHealth);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute(), SaveData.Stats.CurrentBattlePoint);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), SaveData.Stats.CurrentTimePower);
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), SaveData.Stats.CurrentActionGauge);

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] Stats Loaded. HP: %.1f, Level: %.0f"), SaveData.Stats.CurrentHealth, SaveData.Stats.Level);
}

void USPSaveGameSubsystem::ResetSaveData()
{
	SaveData.Reset();
	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] Data Reset."));
}