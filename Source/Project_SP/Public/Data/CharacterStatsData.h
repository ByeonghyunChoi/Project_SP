#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "CharacterStatsData.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FCharacterStatsData : public FTableRowBase
{
	GENERATED_BODY()

public:

    FCharacterStatsData()
        : MaxHealthCap(5000.f)
        , AttackPowerCap(500.f)
        , DefensePowerCap(300.f)
        , iLevel(1)
        , fCurrentHealth(100.0f)
        , fMaxHealth(100.0f)
        , fAttackPower(10.0f)
        , fMovementSpeed(100.0f)
        , fCriticalChance(0.05f)
        , fCriticalDamageMultiplier(1.5f)
        , fStatusEffectResistance(0.0f)
        , fStatusEffectAccuracy(0.0f)
        , fStatusEffectMultiplier(1.0f)
        , fDamageIncreaseMultiplier(0.0f)
        , fDamageReductionMultiplier(0.0f)
        , fLevelPenetration(0.0f)
    {
    }

    // --- 2. 파라미터 생성자 (Parameterized Constructor) ---
    FCharacterStatsData(
        float InMaxHealthCap,
        float InAttackPowerCap,
        float InDefensePowerCap,
        int32 InLevel,
        float InCurrentHealth,
        float InMaxHealth,
        float InAttackPower,
        float InMovementSpeed,
        float InCriticalChance,
        float InCriticalDamageMultiplier,
        float InStatusEffectResistance,
        float InStatusEffectAccuracy,
        float InStatusEffectMultiplier,
        float InDamageIncreaseMultiplier,
        float InDamageReductionMultiplier,
        float InLevelPenetration
    )
        : MaxHealthCap(InMaxHealthCap)
        , AttackPowerCap(InAttackPowerCap)
        , DefensePowerCap(InDefensePowerCap)
        , iLevel(InLevel)
        , fCurrentHealth(InCurrentHealth)
        , fMaxHealth(InMaxHealth)
        , fAttackPower(InAttackPower)
        , fMovementSpeed(InMovementSpeed)
        , fCriticalChance(InCriticalChance)
        , fCriticalDamageMultiplier(InCriticalDamageMultiplier)
        , fStatusEffectResistance(InStatusEffectResistance)
        , fStatusEffectAccuracy(InStatusEffectAccuracy)
        , fStatusEffectMultiplier(InStatusEffectMultiplier)
        , fDamageIncreaseMultiplier(InDamageIncreaseMultiplier)
        , fDamageReductionMultiplier(InDamageReductionMultiplier)
        , fLevelPenetration(InLevelPenetration)
    {
    }

	// --- 성장 시스템용 데이터 (레벨 1 -> 50 성장 폭 설정) ---
	// 레벨이 올랐을 때 도달할 수 있는 최대치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Caps")
	float MaxHealthCap = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Caps")
	float AttackPowerCap = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Caps")
	float DefensePowerCap = 300.f;

	// --- 기본 스탯 (Base Stats) ---

	// 시작 레벨 (보통 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 iLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fMaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fAttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fMovementSpeed = 100.0f; 

	// --- 전투 확률 및 보정 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCriticalChance = 0.05f; // 5% 크확

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCriticalDamageMultiplier = 1.5f; // 150% 크뎀

	// --- 상태 이상 관련 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectResistance = 0.0f; // 상태 저항

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectAccuracy = 0.0f; // 상태 적중

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectMultiplier = 1.0f; // 효과 증가

	// --- 데미지 공식 보정 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDamageIncreaseMultiplier = 0.0f; // 피해량 증가(내가 주는)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDamageReductionMultiplier = 0.0f; // 피해량 감소(내가 받는)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fLevelPenetration = 0.0f; //레벨 무시
};