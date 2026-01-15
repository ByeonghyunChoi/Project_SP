// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SPGASAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PROJECT_SP_API USPGASAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	USPGASAttributeSet();

	// 수치가 변경될 때 제한(Clamping)을 거는 함수
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 데미지 처리 등을 위해 변경 직후 호출되는 함수
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	/** 1. 기본 생존 및 전투 스탯 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData AttackPower; // 공격력
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, AttackPower);

	/** 2. 턴 시스템 관련 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Turn")
	FGameplayAttributeData Speed; // 행동게이지 증가 속도
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Speed);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Turn")
	FGameplayAttributeData ActionGauge; // 현재 행동게이지 (0~100)
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, ActionGauge);

	/** 3. 치명타 관련 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Crit")
	FGameplayAttributeData CritChance; // 치명타 확률
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, CritChance);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Crit")
	FGameplayAttributeData CritDamage; // 치명타 피해량
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, CritDamage);

	/** 4. 피해량 수정 스탯 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | DamageMod")
	FGameplayAttributeData DamageDealtInc; // 가하는 피해량 증가
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, DamageDealtInc);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | DamageMod")
	FGameplayAttributeData DamageDealtDec; // 가하는 피해량 감소
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, DamageDealtDec);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | DamageMod")
	FGameplayAttributeData DamageReceivedInc; // 받는 피해량 증가
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, DamageReceivedInc);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | DamageMod")
	FGameplayAttributeData DamageReceivedDec; // 받는 피해량 감소
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, DamageReceivedDec);

	/** 5. 방어 및 관통 관련 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Defense")
	FGameplayAttributeData Durability; // 내구력
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Durability);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Defense")
	FGameplayAttributeData IgnoreDurability; // 내구력 무시
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, IgnoreDurability);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Defense")
	FGameplayAttributeData IgnoreLevelScaling; // 레벨 무시
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, IgnoreLevelScaling);

	/** 6. 효과(디버프/상태이상) 관련 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Effect")
	FGameplayAttributeData EffectEfficiency; // 효과 효율
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, EffectEfficiency);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Effect")
	FGameplayAttributeData EffectProbability; // 효과 적용 확률
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, EffectProbability);

	/** 7. 성장 및 자원 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Growth")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Level);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Growth")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Experience);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Resource")
	FGameplayAttributeData TimePower; // 시간의 힘
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, TimePower);

	//데미지 스탯(가상)
	/*
	1. 전투 스킬 데미지
	2. 상태 이상 데미지
	3. 패링 공격 데미지
	*/ 
};
