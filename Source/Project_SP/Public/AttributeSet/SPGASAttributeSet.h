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
	// 기본 생존 및 자원 스텟
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData BattlePoint; // 배틀 포인트
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, BattlePoint)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData MaxBattlePoint; // 배틀 포인트 최대 값
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, MaxBattlePoint)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData TimePower; // 시간의 힘
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, TimePower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData MaxTimePower; // 시간의 힘 최대 값
 	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, MaxTimePower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData Defense; // 방어력
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Defense)

	// 속도 및 행동 게이지 스텟
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Basic")
	FGameplayAttributeData Speed; // 속도
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Speed)

	// 행동 게이지 (0 ~ Max)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Turn")
	FGameplayAttributeData ActionGauge;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, ActionGauge)

	// 행동 게이지 최대치 (보통 상수지만 변동 가능성을 위해)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Turn")
	FGameplayAttributeData MaxActionGauge;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, MaxActionGauge)

	// 공격 관련 스텟
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Offense")
	FGameplayAttributeData Attack; // 공격력
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Attack)
 
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Offense")
	FGameplayAttributeData DefenseIgnore; // 방어력 무시
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, DefenseIgnore)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Offense")
	FGameplayAttributeData CriticalRate; // 치명타 확률 
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, CriticalRate)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Offense")
	FGameplayAttributeData CriticalDamage; // 치명타 피해
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, CriticalDamage)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Offense")
	FGameplayAttributeData EffectHitRate; // 효과 명중
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, EffectHitRate)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Offense")
	FGameplayAttributeData EffectAmplify; // 효과 증폭
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, EffectAmplify)

	//피해량 수정 스탯 
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Multiplier")
	FGameplayAttributeData OutgoingDamageMultiplier; // 가하는 피해량 
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, OutgoingDamageMultiplier)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Multiplier")
	FGameplayAttributeData IncomingDamageMultiplier; // 받는 피해량
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, IncomingDamageMultiplier)

	// 성장
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Growth")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Level)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Growth")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, Experience)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Growth")
	FGameplayAttributeData MaxExperience;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, MaxExperience)
	
	//약점 관련 스텟
	// [수르트] 저항력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Resistance")
	FGameplayAttributeData ResistanceSurtr;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, ResistanceSurtr)

	// [펜리르] 저항력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Resistance")
	FGameplayAttributeData ResistanceFenrir;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, ResistanceFenrir)

	// [요르문간드] 저항력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Resistance")
	FGameplayAttributeData ResistanceJormungandr;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, ResistanceJormungandr)

	//메타 속성 스텟
	// 들어오는 데미지 
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, IncomingDamage)

	// 들어오는 힐
	UPROPERTY(BlueprintReadOnly, Category = "Attributes | Meta")
	FGameplayAttributeData IncomingHeal;
	ATTRIBUTE_ACCESSORS(USPGASAttributeSet, IncomingHeal)

};
