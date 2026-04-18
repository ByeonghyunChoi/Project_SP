// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Asset/RelicDefinition.h"
#include "Data/Asset/OpartsDefinition.h"
#include "Map/MapInfo.h"
#include "Engine/DataTable.h"
#include "SPDataStructs.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPlayerLevelRewardRow : public FTableRowBase
{
    GENERATED_BODY()

   // 보상 테이블 혹시 모르니 코드 상에는 남겨 둠
};

USTRUCT(BlueprintType)
struct FPlayerStatsData
{
	GENERATED_BODY()

public:
    UPROPERTY() float CurrentHealth = 163.0f;
    UPROPERTY() float CurrentBattlePoint = 2.0f;
    UPROPERTY() float CurrentTimePower = 200.0f;    
    UPROPERTY() float CurrentActionGauge = 0.0f;

    UPROPERTY() float MaxHealth = 163.0f;
    UPROPERTY() float MaxBattlePoint = 5.0f;
    UPROPERTY() float MaxTimePower = 200.0f;       

    UPROPERTY() float Attack = 84.0f;
    UPROPERTY() float Defense = 52.0f;
    UPROPERTY() float Speed = 100.0f;               

    UPROPERTY() float DefenseIgnore = 0.0f;
    UPROPERTY() float CriticalRate = 0.0f;
    UPROPERTY() float CriticalDamage = 0.0f;
    UPROPERTY() float EffectHitRate = 0.0f;
    UPROPERTY() float EffectAmplify = 0.0f;

    UPROPERTY() float OutgoingDamageMultiplier = 0.0f;
    UPROPERTY() float IncomingDamageMultiplier = 0.0f;

    UPROPERTY() float Level = 7.0f;
    UPROPERTY() float Experience = 0.0f;
    UPROPERTY() float MaxExperience = 3000.0f;

};

USTRUCT(BlueprintType)
struct FPlayerRunWallet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Money = 0; // 골드

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Fragment = 0; // 파편
};

USTRUCT(BlueprintType)
struct FPlayerPermanentWallet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Sand = 1000; // 모래

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 IncompleteEnergy = 10; // 불완전한 기운
};

USTRUCT(BlueprintType)
struct FPlayerRelicData
{
    GENERATED_BODY()

    // 현재 장착 중인 유물
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<const URelicDefinition>> EquippedRelics;

    // 획득했던 이력 (유물 효과 유지용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<const URelicDefinition>> AcquiredHistory;
};

// 개별 오파츠의 성장도를 기억
USTRUCT(BlueprintType)
struct FOpartsProgressData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UnlockedArtifactCount = 0;
};

// 🌟 세이브 파일에 들어갈 오파츠 마스터 데이터
USTRUCT(BlueprintType)
struct FPlayerOpartsData
{
    GENERATED_BODY()

    // 현재 마지막으로 장착하고 있던 오파츠
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<const UOpartsDefinition> EquippedOparts = nullptr;

    // 핵심! 모든 오파츠의 성장도를 기록하는 장부 (Key: 오파츠, Value: 성장도)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<TObjectPtr<const UOpartsDefinition>, FOpartsProgressData> ProgressMap;
};


USTRUCT(BlueprintType)
struct FPlayerMapProgressData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStage = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentFloor = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMapType CurrentMapType = EMapType::NormalBattle;

    // 방을 클리어한 상태에서 저장되었는가? (보상 상자 띄우기 위함)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMapState CurrentRoomState = EMapState::InProgress;

    //로비 맵에서 저장 했는 지 확인(필드 맵과 구분하기 위함)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSavedInLobby = true;

    // 플레이어가 마지막으로 서 있던 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform SavedPlayerTransform;

    //포탈 목적지 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMapType> SavedPortalOptions;
};

USTRUCT(BlueprintType)
struct FPlayerMetaProgressionData //영구 데이터
{
    GENERATED_BODY()

    UPROPERTY()
    FPlayerPermanentWallet PermanentWallet; // 영구적 재화

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerOpartsData OpartsData;

    void Reset()
    {
        // 지갑 0원으로 초기화
        PermanentWallet = FPlayerPermanentWallet();
        // 오파츠 장착 해제 및 장부(Map) 싹 비우기
        OpartsData = FPlayerOpartsData();
    }
};

USTRUCT(BlueprintType)
struct FPlayerRunData // 런 데이터
{
    GENERATED_BODY()

    UPROPERTY()
    FPlayerRunWallet RunWallet; // 일시적 재화

    // 스탯 데이터 포함 (계층 구조)
    UPROPERTY()
    FPlayerStatsData Stats; // 스탯

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerRelicData RelicData; // 유물

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerMapProgressData MapProgress;

    bool IsValid() const { return Stats.CurrentHealth >= 0.0f; }

    // 초기화
    void Reset()
    {
        Stats = FPlayerStatsData();
        RunWallet = FPlayerRunWallet();
        RelicData = FPlayerRelicData();
        MapProgress = FPlayerMapProgressData();
    }
};
