// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Asset/RelicDefinition.h"
#include "GameplayEffectTypes.h"
#include "Data/SPDataStructs.h"
#include "RelicComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API URelicComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URelicComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 최대 장착 가능 개수
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Relic")
    int32 MaxRelicCount = 6;

    // 현재 장착 중인 유물 목록
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Relic")
    TArray<TObjectPtr<const URelicDefinition>> EquippedRelics;

    // 유물이 부여한 고유 GE 핸들 관리 (초기화 시 지우기 위함)
    TArray<FActiveGameplayEffectHandle> RelicEffectHandles;

    // 획득 시 등급별 공격력 보너스를 주기 위한 GE 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Relic|Bonus")
    TSubclassOf<UGameplayEffect> RarityAttackBonusEffectClass;

    // "이번 회차"의 유물 획득 기록 (중복 보너스 방지용)
    UPROPERTY(VisibleAnywhere, Category = "Relic")
    TArray<TObjectPtr<const URelicDefinition>> AcquiredHistory;

	///////////////// 유물 관리 함수들 /////////////////
    // 1. 유물 장착
    UFUNCTION(BlueprintCallable, Category = "Relic")
    bool AddRelic(const URelicDefinition* NewRelic);

    // 2. 유물 해제(버리기)
    UFUNCTION(BlueprintCallable, Category = "Relic")
    bool RemoveRelicAtIndex(int32 SlotIndex);

    // 3. 회차 종료 시 모든 유물 및 스탯 보너스 초기화
    UFUNCTION(BlueprintCallable, Category = "Relic")
    void ResetAllRelics();

    // 4. 스테이지와 전체 유물 목록을 받아, 3개의 유물을 뽑아주는 함수
    UFUNCTION(BlueprintCallable, Category = "Relic")
    TArray<URelicDefinition*> GenerateRelicRewards(int32 CurrentStage, const TArray<URelicDefinition*>& AllRelicPool);

    //세이브 시스템에서 호출할 로드 전용 함수
    UFUNCTION(BlueprintCallable, Category = "Relic")
    void LoadRelicData(const FPlayerRelicData& SavedRelicData);
};
