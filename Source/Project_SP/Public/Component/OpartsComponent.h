// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "Data/Asset/OpartsDefinition.h"
#include "Data/SPDataStructs.h"
#include "OpartsComponent.generated.h"

class UAbilitySystemComponent;

// [런타임 데이터] 게임 도중 변하는 오파츠의 상태 정보
USTRUCT(BlueprintType)
struct FOpartsRuntimeData
{
	GENERATED_BODY()

public:
	// 현재 장착중인 오파츠 정보 (어떤 오파츠인가?)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<const UOpartsDefinition> Definition = nullptr;

	// 현재 레벨 (1~5)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentLevel = 1;

	// 현재 해금된 아티팩트 개수 (0~5)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 UnlockedArtifactCount = 0;

	// [GAS 관리용] 적용된 핸들들 (나중에 해제하거나 레벨업할 때 필요)
	// 1. 기본 패시브 능력 핸들
	FGameplayAbilitySpecHandle PassiveAbilityHandle;
	// 2. 기본 스탯 이펙트 핸들
	FActiveGameplayEffectHandle StatEffectHandle;
	// 3. 아티팩트로 부여된 능력들 핸들 목록
	TArray<FGameplayAbilitySpecHandle> ArtifactAbilityHandles;
	// 4. 아티팩트로 부여된 스탯들 핸들 목록
	TArray<FActiveGameplayEffectHandle> ArtifactEffectHandles;

	void Clear()
	{
		Definition = nullptr;
		CurrentLevel = 1;
		UnlockedArtifactCount = 0;
		PassiveAbilityHandle = FGameplayAbilitySpecHandle();
		StatEffectHandle = FActiveGameplayEffectHandle();
		ArtifactAbilityHandles.Empty();
		ArtifactEffectHandles.Empty();
	}
};
// [델리게이트] 오파츠 상태 변경 시 UI 알림 (장착, 레벨업, 아티팩트 해금 등)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpartsStateChanged, const FOpartsRuntimeData&, RuntimeData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UOpartsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOpartsComponent();

protected:
	virtual void BeginPlay() override;

	// GAS 컴포넌트 캐싱
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	// [핵심] 현재 오파츠의 상태 데이터
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Oparts|Data")
	FOpartsRuntimeData RuntimeData;

	// 교체 가능한 모든 오파츠 목록 (에디터 에서 등록)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oparts|Setup")
	TArray<TObjectPtr<const UOpartsDefinition>> AllOpartsList;

public:
	// UI 업데이트용 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Oparts|Event")
	FOnOpartsStateChanged OnOpartsUpdated;

	//오파츠 부위 별 강화 진행 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts|Data")
	TMap<TObjectPtr<const UOpartsDefinition>, FOpartsProgressData> OpartsProgressMap;

public:
	//다음 오파츠로 교체 ( > 버튼)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void EquipNextOparts();

	//이전 오파츠로 교체 ( < 버튼)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void EquipPreviousOparts();
	
	// 오파츠 장착 (기존의 SetActiveOparts 대체)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void EquipOparts(const UOpartsDefinition* NewOpartsDef);

	// 오파츠 해제
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void UnequipCurrentOparts();

	
	// 오파츠 레벨업 시도 (모래 소모 로직 포함 예정)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void TryUpgradeLevel();

	// 아티팩트 해금 시도 (불완전한 기운 소모 로직 포함 예정)
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void TryUnlockNextArtifact();

	
	UFUNCTION(BlueprintPure, Category = "Oparts")
	const FOpartsRuntimeData& GetCurrentOpartsData() const { return RuntimeData; }

	//데이터 로드 함수
	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void LoadOpartsData(const FPlayerOpartsData& SavedData);

	// [UI 전용] 다음 해금할 아티팩트의 정보(이름, 설명 등) 가져오기
	// 해금할 게 없으면(만렙이면) false 반환
	UFUNCTION(BlueprintPure, Category = "Oparts|UI")
	bool GetNextArtifactData(FOpartsArtifactData& OutArtifactData) const;

	// [UI 전용] 다음 아티팩트 해금에 필요한 '불완전한 기운' 비용 가져오기
	// 해금할 게 없으면 -1 반환
	UFUNCTION(BlueprintPure, Category = "Oparts|UI")
	int32 GetNextArtifactUnlockCost() const;

private:
	// 내부 헬퍼: 현재 상태(레벨, 아티팩트)에 맞춰 GAS 능력/스탯 재적용
	void ApplyOpartsStatsAndAbilities();
};
