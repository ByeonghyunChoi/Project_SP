// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArtifactTypes.h"
#include "ArtifactData.h"
#include "OrpartsData.h"
#include "ArtifactSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatsChanged, const FStatBonus&, NewStats);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UArtifactSystemComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UArtifactSystemComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 아티팩트 장착
    UFUNCTION(BlueprintCallable)
    void EquipArtifact(const FArtifactData& Artifact);

	// 오파츠 장착
    UFUNCTION(BlueprintCallable)
    void EquipOrparts(const FOrpartsData& Orparts);

	// 아티팩트 해제
    UFUNCTION(BlueprintCallable)
    void UnequipArtifact(EArtifactType SlotType);

	// 오파츠 해제
    UFUNCTION(BlueprintCallable)
    void UnequipOrparts();

	// 오파츠 업그레이드
    UFUNCTION(BlueprintCallable)
    void UpgradeOrparts(FName MaterialArtifactID);

	// 총 능력치 계산
    UFUNCTION(BlueprintPure)
    FStatBonus GetTotalStatBonus() const;

	// 현재 장착된 아티팩트 목록
    UFUNCTION(BlueprintPure)
    FArtifactData GetEquippedArtifact(EArtifactType Type) const;

	// 현재 장착된 오파츠 
    UFUNCTION(BlueprintPure)
    FOrpartsData GetEquippedOrparts() const;

    // 능력치 변경 시 알림
    UPROPERTY(BlueprintAssignable, Category = "Stats")
    FOnStatsChanged OnStatsChanged;

private:
    void RecalculateStats();

    UPROPERTY()
    TMap<EArtifactType, FArtifactData> EquippedArtifacts;

    UPROPERTY()
    FOrpartsData EquippedOrparts;

    UPROPERTY()
    bool bHasOrpartsEquipped = false;

    FStatBonus TotalStats;
};
