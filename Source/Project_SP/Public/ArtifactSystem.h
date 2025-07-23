// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArtifactData.h"  
#include "ArtifactSystem.generated.h"


// 아티팩트 장착/해제 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnArtifactEquipped, EArtifactType, ArtifactType, FName, ArtifactID, bool, bEquipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatsChanged, FStatBonus, TotalStats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSetBonusActivated, EArtifactSetType, SetType, int32, PieceCount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class PROJECT_SP_API UArtifactSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArtifactSystem();

protected:
    virtual void BeginPlay() override;

    // 현재 장착된 아티팩트들 (5개 슬롯)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipped Artifacts")
    TMap<EArtifactType, FEquippedArtifact> EquippedArtifacts;

    // 현재 장착된 오파츠
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipped Artifacts")
    FEquippedArtifact EquippedOpartz;

    // 아티팩트 데이터 테이블 참조
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    class UDataTable* ArtifactDataTable;

    // 세트 보너스 데이터 테이블 참조
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    class UDataTable* SetBonusDataTable;

    // 현재 총 스탯 보너스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    FStatBonus TotalStatBonus;

    // 현재 활성화된 세트 보너스들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Set Bonus")
    TMap<EArtifactSetType, int32> ActiveSetCounts;

public:
    // 이벤트 델리게이트들
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnArtifactEquipped OnArtifactEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStatsChanged OnStatsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSetBonusActivated OnSetBonusActivated;

    // 아티팩트 장착 함수
    UFUNCTION(BlueprintCallable, Category = "Artifact System")
    bool EquipArtifact(FName ArtifactID);

    // 아티팩트 해제 함수
    UFUNCTION(BlueprintCallable, Category = "Artifact System")
    bool UnequipArtifact(EArtifactType ArtifactType);

    // 오파츠 장착 함수
    UFUNCTION(BlueprintCallable, Category = "Artifact System")
    bool EquipOpartz(FName OpartzID);

    // 오파츠 해제 함수
    UFUNCTION(BlueprintCallable, Category = "Artifact System")
    bool UnequipOpartz();

    // 특정 슬롯의 장착된 아티팩트 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    FEquippedArtifact GetEquippedArtifact(EArtifactType ArtifactType) const;

    // 장착된 오파츠 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    FEquippedArtifact GetEquippedOpartz() const;

    // 총 스탯 보너스 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    FStatBonus GetTotalStatBonus() const;

    // 특정 세트의 장착된 피스 수 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    int32 GetSetPieceCount(EArtifactSetType SetType) const;

    // 활성화된 세트 보너스 목록 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    TArray<FSetBonusData> GetActiveSetBonuses() const;

    // 아티팩트 데이터 가져오기 (데이터 테이블에서)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    bool GetArtifactData(FName ArtifactID, FArtifactData& OutArtifactData) const;

    // 모든 아티팩트 해제
    UFUNCTION(BlueprintCallable, Category = "Artifact System")
    void UnequipAllArtifacts();

    // 특정 세트의 모든 아티팩트 해제
    UFUNCTION(BlueprintCallable, Category = "Artifact System")
    void UnequipArtifactSet(EArtifactSetType SetType);

    // 아티팩트 장착 가능 여부 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    bool CanEquipArtifact(FName ArtifactID) const;

    // 현재 장착된 모든 아티팩트 정보 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Artifact System")
    TArray<FEquippedArtifact> GetAllEquippedArtifacts() const;

 //   //랜덤 능력치 증가 함수
	//UFUNCTION(BlueprintCallable, Category = "Artifact System")
	//void ApplyArtifactBonusToCharacter(FStatBonus Bonus, FCharacterStatsData& TargetStats);

protected:
    // 스탯 보너스 재계산 함수
    void RecalculateStats();

    // 세트 보너스 재계산 함수
    void RecalculateSetBonuses();

    // 세트 카운트 업데이트 함수
    void UpdateSetCounts();

    // 아티팩트 데이터 테이블에서 데이터 로드
    FArtifactData* LoadArtifactData(FName ArtifactID) const;

    // 세트 보너스 데이터 테이블에서 데이터 로드
    FSetBonusData* LoadSetBonusData(EArtifactSetType SetType, int32 PieceCount) const;
};
