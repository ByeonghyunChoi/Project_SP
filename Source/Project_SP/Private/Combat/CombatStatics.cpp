#include "Combat/CombatStatics.h"
#include "Component/AttributesComponent.h"
#include "Character/CombatPawn.h"

static const float DefenseDivisor = 500.0f;

float UCombatStatics::CalculateDamage(const UAttributesComponent* AttackerStats, const UAttributesComponent* TargetStats, float SkillCoefficient)
{
    if (!AttackerStats || !TargetStats)
    {
        return 0.0f;
    }

    float LevelCoefficient = 1.0f;
    int32 LevelDifference = FMath::Abs(AttackerStats->GetLevel() - TargetStats->GetLevel());

    if (LevelDifference >= 6 && LevelDifference <= 10)
    {
        LevelCoefficient = 0.8f;
    }
    else if (LevelDifference > 10)
    {
        LevelCoefficient = 0.5f;
    }

    float BaseDamage = AttackerStats->GetCurrentStats().fAttackPower * SkillCoefficient;
    float CriticalCoefficient = 1.0f;
    float CriticalChance = AttackerStats->GetCurrentStats().fCriticalChance;
    float RandomFloat = FMath::FRand(); // 0.0 ~ 1.0사이의 값
    if (CriticalChance >= RandomFloat)
    {
        CriticalCoefficient*= AttackerStats->GetCurrentStats().fCriticalDamageMultiplier; // 1.0 * 크뎀
    }
    else
    {
        CriticalCoefficient = 1.0f;
    }
    float DamageMultiCoef = 1.0f + AttackerStats->GetCurrentStats().fDamageIncreaseMultiplier - AttackerStats->GetCurrentStats().fDamageReductionMultiplier;; // 주는 데미지 계수
    // 최종 데미지
    float FinalDamage = BaseDamage * DamageMultiCoef * LevelCoefficient;

    // 최종 데미지를 반올림하여 반환
    return FMath::RoundToFloat(FinalDamage);
}

float UCombatStatics::CalculateStatusEffectDamage(const ACombatPawn* Instigator, const ACombatPawn* Target, const FStatusSubEffect& SubEffect)
{
    if (!Instigator || !Target || !Instigator->GetAttributesComponent() || !Target->GetAttributesComponent())
    {
        return 0.0f;
    }

    const UAttributesComponent* InstigatorStats = Instigator->GetAttributesComponent();
    const UAttributesComponent* TargetStats = Target->GetAttributesComponent();

    float LevelCoefficient = 1.0f;
    int32 LevelDifference = FMath::Abs(InstigatorStats->GetLevel() - TargetStats->GetLevel());

    if (LevelDifference >= 6 && LevelDifference <= 10)
    {
        LevelCoefficient = 0.8f;
    }
    else if (LevelDifference > 10)
    {
        LevelCoefficient = 0.5f;
    }

    // 1. 기본 데미지 계산
    float BaseDamage = InstigatorStats->GetCurrentStats().fAttackPower * SubEffect.EffectMagnitude;

    // 2. 증감 계수 계산
    float IncreaseDamage = (1.0f + InstigatorStats->GetCurrentStats().fDamageIncreaseMultiplier - TargetStats->GetCurrentStats().fDamageReductionMultiplier);

    // 3. 효과 증가 계수 계산
    float StatusEffectMultiCoef = 1.0f + InstigatorStats->GetCurrentStats().fStatusEffectMultiplier;

    // 4. 모든 계수를 곱하여 최종 데미지 계산 (상태 이상 효과 배율 포함)
    float FinalDamage = BaseDamage * IncreaseDamage * LevelCoefficient * StatusEffectMultiCoef;

    //반올림
    return FMath::RoundToFloat(FinalDamage);
}