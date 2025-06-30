// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterStats.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCharacterStatsData
{
	GENERATED_BODY()

	// 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCurrentHealth;
	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fMaxHealth;
	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fAttackPower;
	// 방어력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDefensePower;
	// 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fSpeed;
	// 치명타 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCriticalChance;
	// 치명타 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCriticalDamageMultiplier;
	// 명중치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fAccuracy;
	// 회피치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fEvasion;
	// 상태 저항
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectResistance;
	// 상태 적중
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectAccuracy;
	// 피해량 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDamageIncreaseMultiplier;
	// 피해량 감소
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDamageReductionMultiplier;
	// 방어 무시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fArmorPenetration;

	FCharacterStatsData()
	{
		fMaxHealth = 200.0f;
		fCurrentHealth = fMaxHealth;
		fAttackPower = 100.0f;
		fDefensePower = 80.0f;
		fSpeed = 100.0f;
		fCriticalChance = 0.1f;
		fCriticalDamageMultiplier = 1.5f;
		fAccuracy = 0.0f;
		fEvasion = 0.1f;
		fStatusEffectResistance = 10.0f;
		fStatusEffectAccuracy = 10.0f;
		fDamageIncreaseMultiplier = 0.0f;
		fDamageReductionMultiplier = 0.0f;
		fArmorPenetration = 0.0f;
	}

	bool operator==(const FCharacterStatsData& InCharacterStats) const
	{
		return ((fCurrentHealth == InCharacterStats.fCurrentHealth) &&
			(fMaxHealth == InCharacterStats.fMaxHealth) &&
			(fAttackPower == InCharacterStats.fAttackPower) &&
			(fDefensePower == InCharacterStats.fDefensePower) &&
			(fSpeed == InCharacterStats.fSpeed) &&
			(fCriticalChance == InCharacterStats.fCriticalChance) &&
			(fCriticalDamageMultiplier == InCharacterStats.fCriticalDamageMultiplier) &&
			(fAccuracy == InCharacterStats.fAccuracy) &&
			(fEvasion == InCharacterStats.fEvasion) &&
			(fStatusEffectResistance == InCharacterStats.fStatusEffectResistance) &&
			(fStatusEffectAccuracy == InCharacterStats.fStatusEffectAccuracy) &&
			(fDamageIncreaseMultiplier == InCharacterStats.fDamageIncreaseMultiplier) &&
			(fDamageReductionMultiplier == InCharacterStats.fDamageReductionMultiplier) &&
			(fArmorPenetration == InCharacterStats.fArmorPenetration));
	}

	friend FORCEINLINE uint32 GetTypeHash(const FCharacterStatsData& InCharacterStats)
	{
		uint32 Hash = GetTypeHash(InCharacterStats.fCurrentHealth);
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fMaxHealth));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fAttackPower));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fDefensePower));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fSpeed));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fCriticalChance));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fCriticalDamageMultiplier));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fAccuracy));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fEvasion));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fStatusEffectResistance));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fStatusEffectAccuracy));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fDamageIncreaseMultiplier));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fDamageReductionMultiplier));
		Hash = HashCombine(Hash, GetTypeHash(InCharacterStats.fArmorPenetration));
		return Hash;
	}
};

UCLASS()
class PROJECT_SP_API UCharacterStats : public UObject
{
	GENERATED_BODY()
	
};
