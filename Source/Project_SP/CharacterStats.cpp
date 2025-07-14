// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStats.h"

UCharacterStats::UCharacterStats()
{
	fMaxHealth = 200.0f;
	fCurrentHealth = fMaxHealth;
	fAttackPower = 100.0f;
	fDefensePower = 80.0f;
	fMovementSpeed = 100.0f;
	fCriticalChance = 0.1f;
	fCriticalDamageMultiplier = 1.5f;
	fHitProbability = 0.0f;
	fEvasion = 0.1f;
	fStatusEffectResistance = 10.0f;
	fStatusEffectAccuracy = 10.0f;
	fDamageIncreaseMultiplier = 0.0f;
	fDamageReductionMultiplier = 0.0f;
	fArmorPenetration = 0.0f;
}

float UCharacterStats::GetCurrentHealth() const
{
	return fCurrentHealth;
}

float UCharacterStats::GetMaxHealth() const
{
	return fMaxHealth;
}

float UCharacterStats::GetAttackPower() const
{
	return fAttackPower;
}

float UCharacterStats::GetDefensePower() const
{
	return fDefensePower;
}

float UCharacterStats::GetMovementSpeed() const
{
	return fMovementSpeed;
}

float UCharacterStats::GetCriticalChance() const
{
	return fCriticalChance;
}

float UCharacterStats::GetCriticalDamageMultiplier() const
{
	return fCriticalDamageMultiplier;
}

float UCharacterStats::GetHitProbability() const
{
	return fHitProbability;
}

float UCharacterStats::GetEvasion() const
{
	return fEvasion;
}

float UCharacterStats::GetStatusEffectResistance() const
{
	return fStatusEffectResistance;
}

float UCharacterStats::GetStatusEffectAccuracy() const
{
	return fStatusEffectAccuracy;
}

float UCharacterStats::GetDamageIncreaseMultiplier() const
{
	return fDamageIncreaseMultiplier;
}

float UCharacterStats::GetDamageReductionMultiplier() const
{
	return fDamageReductionMultiplier;
}

float UCharacterStats::GetArmorPenetration() const
{
	return fArmorPenetration;
}

void UCharacterStats::SetCurrentHealth(const float& InCurrentHealth)
{
	fCurrentHealth = InCurrentHealth;
}

void UCharacterStats::SetMaxHealth(const float& InMaxHealth)
{
	fMaxHealth = InMaxHealth;
}

void UCharacterStats::SetAttackPower(const float& InAttackPower)
{
	fAttackPower = InAttackPower;
}

void UCharacterStats::SetDefensePower(const float& InDefensePower)
{
	fDefensePower = InDefensePower;
}

void UCharacterStats::SetMovementSpeed(const float& InMovementSpeed)
{
	fMovementSpeed = InMovementSpeed;
}

void UCharacterStats::SetCriticalChance(const float& InCriticalChance)
{
	fCriticalChance = InCriticalChance;
}

void UCharacterStats::SetCriticalDamageMultiplier(const float& InCriticalDamageMultiplier)
{
	fCriticalDamageMultiplier = InCriticalDamageMultiplier;
}

void UCharacterStats::SetHitProbability(const float& InHitProbability)
{
	fHitProbability = InHitProbability;
}

void UCharacterStats::SetEvasion(const float& InEvasion)
{
	fEvasion = InEvasion;
}

void UCharacterStats::SetStatusEffectResistance(const float& InStatusEffectResistance)
{
	fStatusEffectResistance = InStatusEffectResistance;
}

void UCharacterStats::SetStatusEffectAccuracy(const float& InStatusEffectAccuracy)
{
	fStatusEffectAccuracy = InStatusEffectAccuracy;
}

void UCharacterStats::SetDamageIncreaseMultiplier(const float& InDamageIncreaseMultiplier)
{
	fDamageIncreaseMultiplier = InDamageIncreaseMultiplier;
}

void UCharacterStats::SetDamageReductionMultiplier(const float& InDamageReductionMultiplier)
{
	fDamageReductionMultiplier = InDamageReductionMultiplier;
}

void UCharacterStats::SetArmorPenetration(const float& InArmorPenetration)
{
	fArmorPenetration = InArmorPenetration;
}

void UCharacterStats::CopyFrom(UCharacterStats* OtherStats)
{
	fCurrentHealth = OtherStats->fCurrentHealth;
	fMaxHealth = OtherStats->fMaxHealth;
	fAttackPower = OtherStats->fAttackPower;
	fDefensePower = OtherStats->fDefensePower;
	fMovementSpeed = OtherStats->fMovementSpeed;
	fCriticalChance = OtherStats->fCriticalChance;
	fCriticalDamageMultiplier = OtherStats->fCriticalDamageMultiplier;
	fHitProbability = OtherStats->fHitProbability;
	fEvasion = OtherStats->fEvasion;
	fStatusEffectResistance = OtherStats->fStatusEffectResistance;
	fStatusEffectAccuracy = OtherStats->fStatusEffectAccuracy;
	fDamageIncreaseMultiplier = OtherStats->fDamageIncreaseMultiplier;
	fDamageReductionMultiplier = OtherStats->fDamageReductionMultiplier;
	fArmorPenetration = OtherStats->fArmorPenetration;
}
