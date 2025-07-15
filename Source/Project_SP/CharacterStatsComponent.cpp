// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStatsComponent.h"

// Sets default values for this component's properties
UCharacterStatsComponent::UCharacterStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

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


// Called when the game starts
void UCharacterStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UCharacterStatsComponent::GetCurrentHealth() const
{
	return fCurrentHealth;
}

float UCharacterStatsComponent::GetMaxHealth() const
{
	return fMaxHealth;
}

float UCharacterStatsComponent::GetAttackPower() const
{
	return fAttackPower;
}

float UCharacterStatsComponent::GetDefensePower() const
{
	return fDefensePower;
}

float UCharacterStatsComponent::GetMovementSpeed() const
{
	return fMovementSpeed;
}

float UCharacterStatsComponent::GetCriticalChance() const
{
	return fCriticalChance;
}

float UCharacterStatsComponent::GetCriticalDamageMultiplier() const
{
	return fCriticalDamageMultiplier;
}

float UCharacterStatsComponent::GetHitProbability() const
{
	return fHitProbability;
}

float UCharacterStatsComponent::GetEvasion() const
{
	return fEvasion;
}

float UCharacterStatsComponent::GetStatusEffectResistance() const
{
	return fStatusEffectResistance;
}

float UCharacterStatsComponent::GetStatusEffectAccuracy() const
{
	return fStatusEffectAccuracy;
}

float UCharacterStatsComponent::GetDamageIncreaseMultiplier() const
{
	return fDamageIncreaseMultiplier;
}

float UCharacterStatsComponent::GetDamageReductionMultiplier() const
{
	return fDamageReductionMultiplier;
}

float UCharacterStatsComponent::GetArmorPenetration() const
{
	return fArmorPenetration;
}

void UCharacterStatsComponent::SetCurrentHealth(const float& InCurrentHealth)
{
	fCurrentHealth = FMath::Clamp(InCurrentHealth, 0.0f, fMaxHealth);
}

void UCharacterStatsComponent::SetMaxHealth(const float& InMaxHealth)
{
	fMaxHealth = InMaxHealth;
}

void UCharacterStatsComponent::SetAttackPower(const float& InAttackPower)
{
	fAttackPower = InAttackPower;
}

void UCharacterStatsComponent::SetDefensePower(const float& InDefensePower)
{
	fDefensePower = InDefensePower;
}

void UCharacterStatsComponent::SetMovementSpeed(const float& InMovementSpeed)
{
	fMovementSpeed = InMovementSpeed;
}

void UCharacterStatsComponent::SetCriticalChance(const float& InCriticalChance)
{
	fCriticalChance = InCriticalChance;
}

void UCharacterStatsComponent::SetCriticalDamageMultiplier(const float& InCriticalDamageMultiplier)
{
	fCriticalDamageMultiplier = InCriticalDamageMultiplier;
}

void UCharacterStatsComponent::SetHitProbability(const float& InHitProbability)
{
	fHitProbability = InHitProbability;
}

void UCharacterStatsComponent::SetEvasion(const float& InEvasion)
{
	fEvasion = InEvasion;
}

void UCharacterStatsComponent::SetStatusEffectResistance(const float& InStatusEffectResistance)
{
	fStatusEffectResistance = InStatusEffectResistance;
}

void UCharacterStatsComponent::SetStatusEffectAccuracy(const float& InStatusEffectAccuracy)
{
	fStatusEffectAccuracy = InStatusEffectAccuracy;
}

void UCharacterStatsComponent::SetDamageIncreaseMultiplier(const float& InDamageIncreaseMultiplier)
{
	fDamageIncreaseMultiplier = InDamageIncreaseMultiplier;
}

void UCharacterStatsComponent::SetDamageReductionMultiplier(const float& InDamageReductionMultiplier)
{
	fDamageReductionMultiplier = InDamageReductionMultiplier;
}

void UCharacterStatsComponent::SetArmorPenetration(const float& InArmorPenetration)
{
	fArmorPenetration = InArmorPenetration;
}

void UCharacterStatsComponent::CopyFrom(UCharacterStatsComponent* OtherStats)
{
	if (!OtherStats)
	{
		return;
	}

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


