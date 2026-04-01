// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeSet/SPGAS_DamageCalculation.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Tag/SPGameplayTags.h"


//속성 캡처
struct FDamageStatics
{
	// 공격자(Source) 스탯
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefenseIgnore);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(OutgoingDamageMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Level);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EffectAmplify);

	// 방어자(Target) 스탯
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamageMultiplier);

	FDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, Attack, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, DefenseIgnore, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, CriticalRate, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, CriticalDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, OutgoingDamageMultiplier, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, Level, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, EffectAmplify, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, Defense, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USPGASAttributeSet, IncomingDamageMultiplier, Target, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics DStatics;
	return DStatics;
}

USPGAS_DamageCalculation::USPGAS_DamageCalculation()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseIgnoreDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalRateDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().OutgoingDamageMultiplierDef);
	RelevantAttributesToCapture.Add(DamageStatics().LevelDef);
	RelevantAttributesToCapture.Add(DamageStatics().EffectAmplifyDef);

	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
	RelevantAttributesToCapture.Add(DamageStatics().IncomingDamageMultiplierDef);
}

void USPGAS_DamageCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	//ASC 가져옴
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();

	if (!SourceASC || !TargetASC) return;

	//태그 정보 가져옴
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	//공격 유형을 판별할 태그
	FGameplayTagContainer EffectTags;
	Spec.GetAllAssetTags(EffectTags);

	//스탯 정보 가져옴
	// [공격자 스탯]
	float Attack = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackDef, EvalParams, Attack);
	Attack = FMath::Max<float>(Attack, 0.0f);

	float DefIgnore = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseIgnoreDef, EvalParams, DefIgnore);

	float CritRate = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalRateDef, EvalParams, CritRate);

	float CritDamageVal = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalDamageDef, EvalParams, CritDamageVal);

	float OutgoingMulti = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().OutgoingDamageMultiplierDef, EvalParams, OutgoingMulti);

	float EffectAmplify = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().EffectAmplifyDef, EvalParams, EffectAmplify);

	float SourceLevel = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().LevelDef, EvalParams, SourceLevel);

	// [방어자 스탯]
	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvalParams, Defense);
	Defense = FMath::Max<float>(Defense, 0.0f);

	float IncomingMulti = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().IncomingDamageMultiplierDef, EvalParams, IncomingMulti);

	// Target Level은 캡처 매크로 충돌 방지를 위해 직접 가져옴
	float TargetLevel = TargetASC->GetNumericAttribute(USPGASAttributeSet::GetLevelAttribute());

	// [스킬 계수] (SetByCaller: Data.Damage)
	float Coefficient = Spec.GetSetByCallerMagnitude(SPTags.Data_Damage, false, 1.0f);

	float FinalDamage = 0.0f;
	bool bIsActualCritical = false;

	if (EffectTags.HasTag(SPTags.Damage_Type_Execute))
	{
		FinalDamage = Coefficient;

		UE_LOG(LogTemp, Warning, TEXT("[처형] 일반 몬스터 즉사! 데미지: %.0f"), FinalDamage);
	}
	//상태 이상 데미지
	else if (EffectTags.HasTag(SPTags.Damage_Type_Status))
	{
		// 기본 데미지
		float BaseDamage = Attack * (Coefficient * (1 + EffectAmplify));

		// 피해 증감 계수
		float DamageMultiCoefficient = 1.0f + OutgoingMulti + IncomingMulti;

		// 방어력 계수
		float EffectiveDefense = Defense * FMath::Clamp(1.0f - DefIgnore, 0.0f, 1.0f);
		float DamageReduction = EffectiveDefense / (EffectiveDefense + 1000.0f);
		float DefenseCoefficient = 1.0f - DamageReduction;

		//레벨 계수
		float LevelDiff = SourceLevel - TargetLevel;
		float LevelCoefficient = 1.0f;

		if (LevelDiff >= 10.0f) { LevelCoefficient = 1.2f; }
		else if (LevelDiff >= 6.0f) { LevelCoefficient = 1.1f; }
		else if (LevelDiff >= -5.0f && LevelDiff <= 5.0f) { LevelCoefficient = 1.0f; }
		else if (LevelDiff >= -9.0f) { LevelCoefficient = 0.85f; }
		else { LevelCoefficient = 0.7f; }

		//최종 연산
		FinalDamage = BaseDamage
			* DamageMultiCoefficient
			* DefenseCoefficient
			* LevelCoefficient;

		UE_LOG(LogTemp, Warning, TEXT("상태 이상 데미지: %.0f (Base:%.0f, DmgMod:%.1f, Def:%.2f, Lv:%.1f)"),
			FinalDamage, BaseDamage, DamageMultiCoefficient, DefenseCoefficient, LevelCoefficient);
	}
	//고정 데미지
	else if (EffectTags.HasTag(SPTags.Damage_Type_Fixed))
	{
		// 기본 데미지
		float BaseDamage = Attack * (Coefficient * (1 + EffectAmplify));

		// 피해 증감 계수
		float DamageMultiCoefficient = 1.0f + OutgoingMulti + IncomingMulti;

		//최종 연산
		FinalDamage = BaseDamage
			* DamageMultiCoefficient;

		UE_LOG(LogTemp, Warning, TEXT("고정 데미지: %.0f (Base:%.0f, DmgMod:%.1f)"),
			FinalDamage, BaseDamage, DamageMultiCoefficient);

	}
	else
	{
		// 기본 데미지
		float BaseDamage = Attack * Coefficient;

		// 치명타 계수
		float CriticalCoefficient = 1.0f;
		bool bIsCritical = FMath::RandRange(0.0f, 1.0f) <= CritRate;
		if (bIsCritical)
		{
			CriticalCoefficient = 1.5f + CritDamageVal;
			bIsActualCritical = true;
			UE_LOG(LogTemp, Log, TEXT("치명타!"));
		}

		// 피해 증감 계수
		float DamageMultiCoefficient = 1.0f + OutgoingMulti + IncomingMulti;

		// 방어력 계수
		float EffectiveDefense = Defense * FMath::Clamp(1.0f - DefIgnore, 0.0f, 1.0f);
		float DamageReduction = EffectiveDefense / (EffectiveDefense + 1000.0f);
		float DefenseCoefficient = 1.0f - DamageReduction;

		// 약점 계수
		float WeaknessCoefficient = 1.0f;

		if (SourceTags->HasTag(SPTags.Weapon_Fenrir) &&
			TargetTags->HasTag(SPTags.Weakness_Fenrir))
		{
			WeaknessCoefficient = 1.2f;
			UE_LOG(LogTemp, Log, TEXT("약점 공격(펜리르)"));
		}
		else if (SourceTags->HasTag(SPTags.Weapon_Surtr) &&
			TargetTags->HasTag(SPTags.Weakness_Surtr))
		{
			WeaknessCoefficient = 1.2f;
			UE_LOG(LogTemp, Log, TEXT("약점 공격(수르트)"));
		}
		else if (SourceTags->HasTag(SPTags.Weapon_Jormungandr) &&
			TargetTags->HasTag(SPTags.Weakness_Jormungandr))
		{
			WeaknessCoefficient = 1.2f;
			UE_LOG(LogTemp, Log, TEXT("약점 공격(요르문간드)"));
		}

		// 레벨 계수
		float LevelDiff = SourceLevel - TargetLevel;
		float LevelCoefficient = 1.0f;

		if (LevelDiff >= 10.0f) { LevelCoefficient = 1.2f; }
		else if (LevelDiff >= 6.0f) { LevelCoefficient = 1.1f; }
		else if (LevelDiff >= -5.0f && LevelDiff <= 5.0f) { LevelCoefficient = 1.0f; }
		else if (LevelDiff >= -9.0f) { LevelCoefficient = 0.85f; }
		else { LevelCoefficient = 0.7f; }

		// 최종 연산
		FinalDamage = BaseDamage
			* CriticalCoefficient
			* DamageMultiCoefficient
			* DefenseCoefficient
			* WeaknessCoefficient
			* LevelCoefficient;

		UE_LOG(LogTemp, Warning, TEXT("데미지: %.0f (Base:%.0f, Crit:%.1f, DmgMod:%.1f, Def:%.2f, Weak:%.1f, Lv:%.1f)"),
			FinalDamage, BaseDamage, CriticalCoefficient, DamageMultiCoefficient, DefenseCoefficient, WeaknessCoefficient, LevelCoefficient);
	}

	// 최소 데미지 1 보장
	FinalDamage = FMath::Max<float>(FinalDamage, 1.0f);
	// 메타 속성(IncomingDamage)에 값 누적
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(USPGASAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage)
	);

	//치명타 여부 추가
	if (bIsActualCritical)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(USPGASAttributeSet::GetIncomingIsCriticalAttribute(), EGameplayModOp::Additive, 1.0f)
		);
	}
}

