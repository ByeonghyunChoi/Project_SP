// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "Tag/SPGameplayTags.h"
#include "StatusEffectData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FStatusEffectConfig
{
	GENERATED_BODY()

	// 상태이상 이름 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EffectName;

	// 적용할 GE 클래스 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	// 지속 턴 수 (즉시 발동형은 0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DurationTurns = 2;

	// 지속 데미지 계수 (공격력의 N%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DotDamageCoefficient = 0.0f;

	// 즉시 발동 데미지 계수 (혼절, 치명상)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InstantDamageCoefficient = 0.0f;

	// 이 상태이상이 즉시 발동인지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsInstantEffect = false;
};

UCLASS()
class PROJECT_SP_API UStatusEffectData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 태그를 키로 사용하여 설정 찾기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Config")
	TMap<FGameplayTag, FStatusEffectConfig> StatusConfigs;

	// 태그로 설정 가져오기
	const FStatusEffectConfig* GetConfig(FGameplayTag Tag) const
	{
		return StatusConfigs.Find(Tag);
	}
};
