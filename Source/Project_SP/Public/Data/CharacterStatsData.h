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
	// 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCurrentHealth = 0.0f;
	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fMaxHealth = 0.0f;
	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fAttackPower = 0.0f;
	// 방어력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDefensePower = 0.0f;
	// 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fMovementSpeed = 0.0f;
	// 치명타 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCriticalChance = 0.0f;
	// 치명타 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fCriticalDamageMultiplier = 0.0f;
	// 명중치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fHitProbability = 0.0f;
	// 회피치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fEvasion = 0.0f;
	// 상태 저항
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectResistance = 0.0f;
	// 상태 적중
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectAccuracy = 0.0f;
	// 피해량 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDamageIncreaseMultiplier = 0.0f;
	// 피해량 감소
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fDamageReductionMultiplier = 0.0f;
	// 방어 무시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fArmorPenetration = 0.0f;
	// 효과 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float fStatusEffectMultiplier = 0.0f;
};