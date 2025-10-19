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
    float DamageMultiCoef = (1 + AttackerStats->GetCurrentStats().fDamageIncreaseMultiplier - AttackerStats->GetCurrentStats().fDamageReductionMultiplier);
    float DefendCoef = 1 - (TargetStats->GetCurrentStats().fDefensePower / (TargetStats->GetCurrentStats().fDefensePower + DefenseDivisor)) + AttackerStats->GetCurrentStats().fArmorPenetration;
    float HitChance = 1 - (TargetStats->GetCurrentStats().fEvasion - AttackerStats->GetCurrentStats().fHitProbability);
    if (FMath::FRand() > HitChance)
    {
        BaseDamage *= 0.5;
    }
    // 최종 데미지
    float FinalDamage = BaseDamage * DamageMultiCoef * DefendCoef * LevelCoefficient;

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

    // 3. 방어 계수 계산
    float defenceCoefficient = 1.0f - (TargetStats->GetCurrentStats().fDefensePower / (TargetStats->GetCurrentStats().fDefensePower + DefenseDivisor)) + InstigatorStats->GetCurrentStats().fArmorPenetration;

    // 4. 모든 계수를 곱하여 최종 데미지 계산 (상태 이상 효과 배율 포함)
    float FinalDamage = BaseDamage * IncreaseDamage * defenceCoefficient * LevelCoefficient * InstigatorStats->GetCurrentStats().fStatusEffectMultiplier;

    return FMath::RoundToFloat(FinalDamage);
}