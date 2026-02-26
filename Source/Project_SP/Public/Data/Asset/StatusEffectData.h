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

	// 1. 상태이상 이름 (디버깅용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EffectName;

	// 2. 적용할 GE 클래스 (스탯 감소용: 방어력, 속도 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	// 3. 지속 턴 수 (즉시 발동형은 0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DurationTurns = 2;

	// 4. 지속 데미지 계수 (공격력의 N%) - 예: 0.2, 0.45
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DotDamageCoefficient = 0.0f;

	// 5. 즉시 발동 데미지 계수 (혼절, 치명상 등) - 예: 1.2, 1.5
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InstantDamageCoefficient = 0.0f;

	// 6. 이 상태이상이 '즉시 발동형(Instant)'인지 여부 (True면 GE 지속시간 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsInstantEffect = false;
};

UCLASS()
class PROJECT_SP_API UStatusEffectData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 태그를 키(Key)로 사용하여 설정을 찾습니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Config")
	TMap<FGameplayTag, FStatusEffectConfig> StatusConfigs;

	// 헬퍼 함수: 태그로 설정 가져오기
	const FStatusEffectConfig* GetConfig(FGameplayTag Tag) const
	{
		return StatusConfigs.Find(Tag);
	}
};
