#include "Combat/CombatStatics.h"
#include "Combat/CharacterStatsComponent.h"
#include "Combat/CombatPawn.h"

float UCombatStatics::CalculateDamage(const UCharacterStatsComponent* AttackerStats, const UCharacterStatsComponent* TargetStats, float SkillCoefficient)
{
    if (!AttackerStats || !TargetStats)
    {
        return 0.0f;
    }

    float BaseDamage = AttackerStats->GetAttackPower() * SkillCoefficient;
    float DamageMultiCoef = (1 + AttackerStats->GetDamageIncreaseMultiplier() - AttackerStats->GetDamageReductionMultiplier());
    float DefendCoef = 1 - (TargetStats->GetDefensePower() / (TargetStats->GetDefensePower() + 500)) + AttackerStats->GetArmorPenetration();
    float HitChance = 1 - (TargetStats->GetEvasion() - AttackerStats->GetHitProbability());
    if (FMath::FRand() > HitChance)
    {
        BaseDamage *= 0.5;
    }
    if (FMath::FRand() < AttackerStats->GetCriticalChance())
    {
        BaseDamage *= AttackerStats->GetCriticalDamageMultiplier();
    }
    //레벨 계수 추가해야 함(모르고 빼먹음;;;)
    float FinalDamage = BaseDamage * DamageMultiCoef * DefendCoef;

    // 최종 데미지를 반올림하여 반환
    return FMath::RoundToFloat(FinalDamage);
}

float UCombatStatics::CalculateStatusEffectDamage(const ACombatPawn* Instigator, const ACombatPawn* Target, const FStatusSubEffect& SubEffect)
{
    if (!Instigator || !Target || !Instigator->GetStatsComponent() || !Target->GetStatsComponent())
    {
        return 0.0f;
    }

    const UCharacterStatsComponent* InstigatorStats = Instigator->GetStatsComponent();
    const UCharacterStatsComponent* TargetStats = Target->GetStatsComponent();

    // 1. 기본 데미지 계산
    float BaseDamage = InstigatorStats->GetAttackPower() * SubEffect.EffectMagnitude;

    // 2. 증감 계수 계산
    float IncreaseDamage = (1.0f + InstigatorStats->GetDamageIncreaseMultiplier() - TargetStats->GetDamageReductionMultiplier());

    // 3. 방어 계수 계산
    float defenceCoefficient = 1.0f - (TargetStats->GetDefensePower() / (TargetStats->GetDefensePower() + 500.0f)) + InstigatorStats->GetArmorPenetration();

    // 4. 모든 계수를 곱하여 최종 데미지 계산 (상태 이상 효과 배율 포함)
    float FinalDamage = BaseDamage * IncreaseDamage * defenceCoefficient * InstigatorStats->GetStatusEffectMultiplier();

    return FMath::RoundToFloat(FinalDamage);
}