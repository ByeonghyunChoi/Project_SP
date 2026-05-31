// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Asset/RelicDefinition.h"
#include "Data/Asset/OpartsDefinition.h"
#include "Map/MapInfo.h"
#include "Engine/DataTable.h"
#include "Data/CombatEncounterData.h"
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
struct FSavedEncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CombatLevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnemySpawnInfo> EnemyGroup;
};

USTRUCT(BlueprintType)
struct FPlayerStatsData
{
	GENERATED_BODY()

public:
    UPROPERTY() float CurrentHealth = 163.0f;
    UPROPERTY() float CurrentBattlePoint = 2.0f;
    UPROPERTY() float CurrentTimePower = 100.0f;    
    UPROPERTY() float CurrentActionGauge = 0.0f;

    UPROPERTY() float MaxHealth = 163.0f;
    UPROPERTY() float MaxBattlePoint = 5.0f;
    UPROPERTY() float MaxTimePower = 100.0f;       

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

    UPROPERTY() float Level = 1.0f;
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
    int32 IncompleteEnergy = 0; // 불완전한 기운
};

USTRUCT(BlueprintType)
struct FPlayerPermanentWallet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Sand = 0; // 모래

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Fragment = 0; // 파편
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SavedFieldLevelName;

    // 튜토리얼 1차전 클리어 여부 세이브
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsTutorialBasicCleared = false;

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

    //몬스터 명부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<int32, FSavedEncounterData> PreGeneratedEncounters;
};

USTRUCT(BlueprintType)
struct FSoundSettingsData
{
    GENERATED_BODY()

    // UI 슬라이더 값 기준 (0.0 ~ 10.0) 기본값은 10으로 세팅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float MasterVolume = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float BGMVolume = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SFXVolume = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float UIVolume = 10.0f;
};

// 권능 수복 능력 리스트
UENUM(BlueprintType)
enum class EPowerUpgradeType : uint8
{
    None			UMETA(DisplayName = "없음"),
    CombatResource	UMETA(DisplayName = "전투 자원 강화 (BP 증가)"),
    RelicReroll		UMETA(DisplayName = "유물 리롤 해금"),
    RewardBoost		UMETA(DisplayName = "보상 강화 (획득량 증가)"),
    TimeSkillCost	UMETA(DisplayName = "시간 간섭 코스트 감소"),
    MaxTimePower	UMETA(DisplayName = "시간의 힘 최대치 증가"),
    FreeRevive		UMETA(DisplayName = "생존 (1회 무료 부활)"),
    StartGold		UMETA(DisplayName = "경제 (시작 골드 획득)"),
    RelicLuck		UMETA(DisplayName = "보상 운 (유물 등장 확률 증가)")
};

// 권능 수복 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FPowerUpgradeData : public FTableRowBase
{
    GENERATED_BODY()

    // 1. 어떤 능력인가?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUpgrade")
    EPowerUpgradeType UpgradeType = EPowerUpgradeType::None;

    // 2. 능력 이름 (예: "전투 시작 BP 최소/최대 증가")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUpgrade")
    FString UpgradeName;

    // 3. 최대 가능 단계 (예: 2단계, 3단계)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUpgrade")
    int32 MaxLevel = 1;

    // 4. 레벨별 파편 소모량 (Index 0: 1단계 비용, Index 1: 2단계 비용...)
    // 예: [10, 20] 넣으면 1단계 갈 때 10, 2단계 갈 때 20 소모
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUpgrade")
    TArray<int32> CostPerLevel;

    // 5. 레벨별 실제 적용될 수치 (Index 0: 1단계 효과, Index 1: 2단계 효과...)
    // 예: 시작 골드면 [100, 300, 500] 입력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUpgrade")
    TArray<float> EffectValuePerLevel;

    // 6. 툴팁이나 비고란 설명 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUpgrade")
    FText Description;
};

USTRUCT(BlueprintType)
struct FPlayerPowerUpgradeData
{
    GENERATED_BODY()

    // 어떤 권능(Enum)을 몇 레벨(int32)까지 올렸는지 영구 저장합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EPowerUpgradeType, int32> UpgradeLevels;
};

USTRUCT(BlueprintType)
struct FPlayerMetaProgressionData //영구 데이터
{
    GENERATED_BODY()

    UPROPERTY()
    FPlayerPermanentWallet PermanentWallet; // 영구적 재화

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerOpartsData OpartsData;

	// 권능 수복
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerPowerUpgradeData PowerUpgradeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
    FSoundSettingsData SoundSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save | Flow")
    bool bHasSeenIntro = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save | Flow")
    bool bHasSeenIntro2 = false;

    void Reset()
    {
        // 지갑 0원으로 초기화
        PermanentWallet = FPlayerPermanentWallet();
        // 오파츠 장착 해제 및 장부(Map) 싹 비우기
        OpartsData = FPlayerOpartsData();
		// 권능 수복 데이터 초기화
        PowerUpgradeData = FPlayerPowerUpgradeData();
        // 인트로 컷신 초기화
        bHasSeenIntro = false;
        bHasSeenIntro2 = false;
    }
};

USTRUCT(BlueprintType)
struct FPlayerRunData // 런 데이터
{
    GENERATED_BODY()

    // 런이 정상적으로 시작되었는지 확인하는 전용 스위치
    UPROPERTY()
    bool bIsRunStarted = false;

    UPROPERTY()
    FPlayerRunWallet RunWallet; // 일시적 재화

    // 스탯 데이터 포함 (계층 구조)
    UPROPERTY()
    FPlayerStatsData Stats; // 스탯

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerRelicData RelicData; // 유물

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPlayerMapProgressData MapProgress;

    bool IsValid() const { return  Stats.CurrentHealth >= 0.0f; }

    // 초기화
    void Reset()
    {
        Stats = FPlayerStatsData();
        RunWallet = FPlayerRunWallet();
        RelicData = FPlayerRelicData();
        MapProgress = FPlayerMapProgressData();
        bIsRunStarted = false;
    }
};