// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CharacterStatsComponent.h"

// Sets default values for this component's properties
UCharacterStatsComponent::UCharacterStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UCharacterStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeStatsFromDataTable();
}

void UCharacterStatsComponent::InitializeStatsFromDataTable()
{
	if (CharacterStatsDataTable)
	{
		FCharacterStatsData* FoundStats = CharacterStatsDataTable->FindRow<FCharacterStatsData>(RowName, TEXT("Looking for Character Stats"));
		if (FoundStats)
		{
			// 찾은 스탯 데이터를 CurrentStats에 복사합니다.
			CurrentStats = *FoundStats;
			UE_LOG(LogTemp, Warning, TEXT("Stats for %s initialized from DataTable (Row: %s). MaxHealth: %f"), *GetOwner()->GetName(), *RowName.ToString(), CurrentStats.fMaxHealth);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to find row %s in DataTable %s."), *RowName.ToString(), *CharacterStatsDataTable->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterStatsDataTable is not set on %s."), *GetOwner()->GetName());
	}
}

float UCharacterStatsComponent::GetCurrentHealth() const
{
	return CurrentStats.fCurrentHealth;
}

float UCharacterStatsComponent::GetMaxHealth() const
{
	return CurrentStats.fMaxHealth;
}

float UCharacterStatsComponent::GetAttackPower() const
{
	return CurrentStats.fAttackPower;
}

float UCharacterStatsComponent::GetDefensePower() const
{
	return CurrentStats.fDefensePower;
}

float UCharacterStatsComponent::GetMovementSpeed() const
{
	return CurrentStats.fMovementSpeed;
}

float UCharacterStatsComponent::GetCriticalChance() const
{
	return CurrentStats.fCriticalChance;
}

float UCharacterStatsComponent::GetCriticalDamageMultiplier() const
{
	return CurrentStats.fCriticalDamageMultiplier;
}

float UCharacterStatsComponent::GetHitProbability() const
{
	return CurrentStats.fHitProbability;
}

float UCharacterStatsComponent::GetEvasion() const
{
	return CurrentStats.fEvasion;
}

float UCharacterStatsComponent::GetStatusEffectResistance() const
{
	return CurrentStats.fStatusEffectResistance;
}

float UCharacterStatsComponent::GetStatusEffectAccuracy() const
{
	return CurrentStats.fStatusEffectAccuracy;
}

float UCharacterStatsComponent::GetDamageIncreaseMultiplier() const
{
	return CurrentStats.fDamageIncreaseMultiplier;
}

float UCharacterStatsComponent::GetDamageReductionMultiplier() const
{
	return CurrentStats.fDamageReductionMultiplier;
}

float UCharacterStatsComponent::GetArmorPenetration() const
{
	return CurrentStats.fArmorPenetration;
}

float UCharacterStatsComponent::GetCurrentSP() const
{
	return fCurrentSP;
}

float UCharacterStatsComponent::GetMaxSP() const
{
	return fMaxSP;
}

void UCharacterStatsComponent::SetCurrentHealth(const float& InCurrentHealth)
{
	CurrentStats.fCurrentHealth = FMath::Clamp(InCurrentHealth, 0.0f, CurrentStats.fMaxHealth);
}

void UCharacterStatsComponent::SetMaxHealth(const float& InMaxHealth)
{
	CurrentStats.fMaxHealth = InMaxHealth;
}

void UCharacterStatsComponent::SetAttackPower(const float& InAttackPower)
{
	CurrentStats.fAttackPower = InAttackPower;
}

void UCharacterStatsComponent::SetDefensePower(const float& InDefensePower)
{
	CurrentStats.fDefensePower = InDefensePower;
}

void UCharacterStatsComponent::SetMovementSpeed(const float& InMovementSpeed)
{
	CurrentStats.fMovementSpeed = InMovementSpeed;
}

void UCharacterStatsComponent::SetCriticalChance(const float& InCriticalChance)
{
	CurrentStats.fCriticalChance = InCriticalChance;
}

void UCharacterStatsComponent::SetCriticalDamageMultiplier(const float& InCriticalDamageMultiplier)
{
	CurrentStats.fCriticalDamageMultiplier = InCriticalDamageMultiplier;
}

void UCharacterStatsComponent::SetHitProbability(const float& InHitProbability)
{
	CurrentStats.fHitProbability = InHitProbability;
}

void UCharacterStatsComponent::SetEvasion(const float& InEvasion)
{
	CurrentStats.fEvasion = InEvasion;
}

void UCharacterStatsComponent::SetStatusEffectResistance(const float& InStatusEffectResistance)
{
	CurrentStats.fStatusEffectResistance = InStatusEffectResistance;
}

void UCharacterStatsComponent::SetStatusEffectAccuracy(const float& InStatusEffectAccuracy)
{
	CurrentStats.fStatusEffectAccuracy = InStatusEffectAccuracy;
}

void UCharacterStatsComponent::SetDamageIncreaseMultiplier(const float& InDamageIncreaseMultiplier)
{
	CurrentStats.fDamageIncreaseMultiplier = InDamageIncreaseMultiplier;
}

void UCharacterStatsComponent::SetDamageReductionMultiplier(const float& InDamageReductionMultiplier)
{
	CurrentStats.fDamageReductionMultiplier = InDamageReductionMultiplier;
}

void UCharacterStatsComponent::SetArmorPenetration(const float& InArmorPenetration)
{
	CurrentStats.fArmorPenetration = InArmorPenetration;
}

void UCharacterStatsComponent::SetCurrentSP(const float& InSP)
{
	fCurrentSP = InSP;
}

void UCharacterStatsComponent::ModifySP(const float& Delta)
{
	fMaxSP = Delta;
}

void UCharacterStatsComponent::CopyFrom(UCharacterStatsComponent* OtherStats)
{
	if (!OtherStats)
	{
		return;
	}

	CurrentStats.fCurrentHealth = OtherStats->CurrentStats.fCurrentHealth;
	CurrentStats.fMaxHealth = OtherStats->CurrentStats.fMaxHealth;
	CurrentStats.fAttackPower = OtherStats->CurrentStats.fAttackPower;
	CurrentStats.fDefensePower = OtherStats->CurrentStats.fDefensePower;
	CurrentStats.fMovementSpeed = OtherStats->CurrentStats.fMovementSpeed;
	CurrentStats.fCriticalChance = OtherStats->CurrentStats.fCriticalChance;
	CurrentStats.fCriticalDamageMultiplier = OtherStats->CurrentStats.fCriticalDamageMultiplier;
	CurrentStats.fHitProbability = OtherStats->CurrentStats.fHitProbability;
	CurrentStats.fEvasion = OtherStats->CurrentStats.fEvasion;
	CurrentStats.fStatusEffectResistance = OtherStats->CurrentStats.fStatusEffectResistance;
	CurrentStats.fStatusEffectAccuracy = OtherStats->CurrentStats.fStatusEffectAccuracy;
	CurrentStats.fDamageIncreaseMultiplier = OtherStats->CurrentStats.fDamageIncreaseMultiplier;
	CurrentStats.fDamageReductionMultiplier = OtherStats->CurrentStats.fDamageReductionMultiplier;
	CurrentStats.fArmorPenetration = OtherStats->CurrentStats.fArmorPenetration;
}


