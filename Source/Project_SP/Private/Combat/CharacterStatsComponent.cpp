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
			// 찾은 스탯 데이터를 BaseStats와 CurrentStats 둘 다에 복사
			BaseStats = *FoundStats; // 원본 스탯 저장
			CurrentStats = *FoundStats; // 현재 스탯으로 사용
			CurrentStats.fCurrentHealth = CurrentStats.fMaxHealth;
			iCurrentLevel = BaseStats.iLevel;

			UE_LOG(LogTemp, Error, TEXT("!!! STATS INITIALIZED for %s. Level: %d, Health set to MAX: %f"), *GetOwner()->GetName(), iCurrentLevel, CurrentStats.fCurrentHealth);
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

void UCharacterStatsComponent::RecalculateStatsForLevelUp(int32 NewLevel)
{
	const int32 MaxLevel = 50;
	if (NewLevel <= 1 || NewLevel > MaxLevel) return;

	// 체력 계산
	float NewMaxHealth = BaseStats.fMaxHealth + (fMaxHealthCap - BaseStats.fMaxHealth) / (MaxLevel - 1) * (NewLevel - 1);
	SetMaxHealth(FMath::RoundToFloat(NewMaxHealth));

	// 공격력 계산
	float NewAttackPower = BaseStats.fAttackPower + (fAttackPowerCap - BaseStats.fAttackPower) / (MaxLevel - 1) * (NewLevel - 1);
	SetAttackPower(FMath::RoundToFloat(NewAttackPower));

	// 방어력 계산
	float NewDefensePower = BaseStats.fDefensePower + (fDefensePowerCap - BaseStats.fDefensePower) / (MaxLevel - 1) * (NewLevel - 1);
	SetDefensePower(FMath::RoundToFloat(NewDefensePower));

	// 속도 계산
	float NewMovementSpeed = BaseStats.fMovementSpeed + (fMovementSpeedCap - BaseStats.fMovementSpeed) / (MaxLevel - 1) * (NewLevel - 1);
	SetMovementSpeed(FMath::RoundToFloat(NewMovementSpeed));

	// 치명타 확률 계산
	float NewCriticalChance = BaseStats.fCriticalChance + (fCriticalChanceCap - BaseStats.fCriticalChance) / (MaxLevel - 1) * (NewLevel - 1);
	SetCriticalChance(NewCriticalChance);

	// 레벨 업데이트
	SetCharacterLevel(NewLevel);
}

int32 UCharacterStatsComponent::GetCharacterLevel() const
{
	return iCurrentLevel;
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

float UCharacterStatsComponent::GetStatusEffectMultiplier() const
{
	return CurrentStats.fStatusEffectMultiplier;
}

void UCharacterStatsComponent::SetCharacterLevel(const int32& InLevel)
{
	iCurrentLevel = InLevel;
}

void UCharacterStatsComponent::SetCurrentHealth(const float& InCurrentHealth)
{
	CurrentStats.fCurrentHealth = FMath::Clamp(InCurrentHealth, 0.0f, CurrentStats.fMaxHealth);
	UE_LOG(LogTemp, Log, TEXT("health changed: %f"), CurrentStats.fCurrentHealth);
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

void UCharacterStatsComponent::SetStatusEffectMultiplier(const float& InStatusEffectMultiplier)
{
	CurrentStats.fStatusEffectMultiplier = InStatusEffectMultiplier;
}

void UCharacterStatsComponent::ModifySP(const float& Delta)
{
	fCurrentSP = FMath::Clamp(fCurrentSP + Delta, 0.0f, fMaxSP);
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