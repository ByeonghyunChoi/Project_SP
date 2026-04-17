// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "OpartsDefinition.generated.h"

/**
 * [아티팩트 데이터 구조체]
 * 오파츠에 종속된 5개의 아티팩트 슬롯 각각에 대한 정의
 * 해금 시 능력(GA)을 주거나, 스탯(GE)을 올려줄 수 있음
 */
USTRUCT(BlueprintType)
struct FOpartsArtifactData
{
	GENERATED_BODY()

public:
	// UI 표시용: 아티팩트 이름 (예: "기묘한 수정 조각")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText ArtifactName;

	// UI 표시용: 효과 설명 (예: "치명타 확률 +20%")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// UI 표시용: 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> Icon;

	// [GAS] 이 아티팩트 해금 시 부여할 패시브 능력 (없으면 None)
	// 예: 크리티컬 차지 메커니즘 변경, 시간 간섭 스킬 변경 등
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> ArtifactAbilityClass;

	// [GAS] 이 아티팩트 해금 시 적용할 스탯 보너스 (없으면 None)
	// 예: 치명타 확률 +20%, 공격력 +20% (Infinite Duration GE 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> ArtifactStatEffectClass;
};

UCLASS(BlueprintType, Const)
class PROJECT_SP_API UOpartsDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 1. UI 표시용 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display")
	FText DisplayName; // 예: "수정해골"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display")
	UTexture2D* Icon; // 오파츠 아이콘

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display", meta = (MultiLine = true))
	FText Description; // 해당하는 오파츠 전체 설명


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display")
	FText UniqueAbilityName; // 예: "치명타 특화"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display", meta = (MultiLine = true))
	FText UniqueAbilityDesc; // 예: "치명타 피해량 +20%"

	// 오파츠 식별 태그 (예: Item.Oparts.CrystalSkull)
	// 장착 시 캐릭터에게 이 태그를 부착하여, 특정 상호작용이나 로직 분기에 사용합니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display")
	FGameplayTag OpartsTag;

	// [GAS] 오파츠 고유 패시브 능력 (장착 즉시 부여)
	// 예: 수정해골의 "크리티컬 차지" 시스템
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "2. Base Ability")
	TSubclassOf<UGameplayAbility> BasePassiveAbility;

	// [GAS] 오파츠 고유 시간 간섭 능력 (장착 즉시 부여)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "2. Base Ability")
	TSubclassOf<UGameplayAbility> TimeInterferenceAbility;

	// [GAS] 레벨별 기본 스탯 (HP, ATK, SPD 등)
	// Infinite Duration GE를 사용하며, GE 내부의 CurveTable을 통해 레벨(1~5)별 수치를 정의
	// 예: Lv1 -> HP+140, Lv5 -> HP+700
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "2. Base Ability")
	TSubclassOf<UGameplayEffect> BaseStatEffectClass;

	// 총 5개의 아티팩트 슬롯 정의
	// 인덱스 0: 1단계 해금, 인덱스 4: 5단계 해금
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "3. Artifacts")
	TArray<FOpartsArtifactData> Artifacts;

	// 데이터 유효성 검사 (에디터 편의성)
	virtual void PostLoad() override
	{
		Super::PostLoad();
		// 아티팩트는 항상 5개여야 하므로, 부족하면 채워넣거나 경고를 띄울 수 있음
	}
};
