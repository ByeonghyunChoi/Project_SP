// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

// 캐릭터의 현재 행동 상태
UENUM(BlueprintType)
enum class ECombatPawnState : uint8
{
    Idle            UMETA(DisplayName = "대기"),
    AwaitingInput UMETA(DisplayName = "입력 대기 중"),
    PerformingAction UMETA(DisplayName = "행동 수행 중"),
    Defeated        UMETA(DisplayName = "사망"),
};

// 캐릭터의 소속 진영
UENUM(BlueprintType)
enum class EFaction : uint8
{
    Player  UMETA(DisplayName = "플레이어"),
    Enemy   UMETA(DisplayName = "적"),
    None    UMETA(DisplayName = "없음")
};

// 턴의 종류
UENUM(BlueprintType)
enum class ETurnType : uint8
{
    Normal      UMETA(DisplayName = "일반 턴"),
    Interrupt   UMETA(DisplayName = "중단 턴")
};

// 패링 결과
UENUM(BlueprintType)
enum class EParryResult : uint8
{
    None,           // 패링 시도 없음
    Success,        // 패링 성공 (약점 일치)
    PartialSuccess  // 부분 성공 (약점 불일치)
};

// 전투의 전체적인 상태
UENUM(BlueprintType)
enum class EBattleState : uint8
{
    Setup UMETA(DisplayName = "전투 준비"),
    InProgress UMETA(DisplayName = "전투 진행 중"),
    Ended UMETA(DisplayName = "전투 종료")
};

UENUM(BlueprintType)
enum class EDamageFloaterType : uint8
{
    Normal        UMETA(DisplayName = "일반 데미지"),
    Critical    UMETA(DisplayName = "치명타 데미지"),
    StatusEffect    UMETA(DisplayName = "상태 이상 데미지")
};

UENUM(BlueprintType)
enum class EMapType : uint8
{
    // "일반" 등급 맵 유형
    Rest            UMETA(DisplayName = "쉼터"),
    NormalBattle    UMETA(DisplayName = "일반 전투"),

    // "에픽" 등급 맵 유형
    StrongEnemyBattle UMETA(DisplayName = "강적 전투"),
    Jester            UMETA(DisplayName = "광대 맵"),

    // "준비" 등급 맵 유형
    Prepare           UMETA(DisplayName = "준비 맵"),

    // "보스" 등급 맵 유형
    BossBattle        UMETA(DisplayName = "보스 전투")
};

UENUM(BlueprintType)
enum class EMapState : uint8
{
    InProgress UMETA(DisplayName = "맵 진행 중"),
    Reward UMETA(DisplayName = "보상 수령 중"),
    Cleard UMETA(DisplatName = "맵 클리어")
};


