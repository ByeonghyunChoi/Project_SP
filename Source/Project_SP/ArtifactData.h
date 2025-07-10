// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ArtifactData.generated.h"

// 아티팩트 타입 열거형 (5개 아티팩트 슬롯)
UENUM(BlueprintType)
enum class EArtifactType : uint8
{
    None        UMETA(DisplayName = "None"),
    Flower      UMETA(DisplayName = "Flower"),      // 체력 증가 (생명의 꽃)
    Feather     UMETA(DisplayName = "Feather"),     // 공격력 증가 (죽음의 깃털)
    Hourglass   UMETA(DisplayName = "Hourglass"),   // 방어력 증가 (시간의 모래)
    Goblet      UMETA(DisplayName = "Goblet"),      // 속도 증가 (공간의 성배)
    Circlet     UMETA(DisplayName = "Circlet")      // 특수 능력 (이성의 왕관)
};

// 아티팩트 세트 타입 열거형 (3개 세트)
UENUM(BlueprintType)
enum class EArtifactSetType : uint8
{
    None        UMETA(DisplayName = "None"),
    Crystal     UMETA(DisplayName = "Crystal Set"),     // 크리스탈 세트
    Emerald     UMETA(DisplayName = "Emerald Set"),     // 옥시계 세트
    Gold        UMETA(DisplayName = "Gold Set")         // 골드 버그 세트
};

// 스탯 증가 구조체
USTRUCT(BlueprintType)
struct FStatBonus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HP = 0.0f;                    // 체력 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Attack = 0.0f;                // 공격력 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Defense = 0.0f;               // 방어력 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Speed = 0.0f;                 // 속도 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Special = 0.0f;               // 특수 능력 증가량

    // 스탯 보너스 합산 함수
    FStatBonus operator+(const FStatBonus& Other) const
    {
        FStatBonus Result;
        Result.HP = HP + Other.HP;
        Result.Attack = Attack + Other.Attack;
        Result.Defense = Defense + Other.Defense;
        Result.Speed = Speed + Other.Speed;
        Result.Special = Special + Other.Special;
        return Result;
    }
};

// 아티팩트 정보 구조체 (데이터 테이블용)
USTRUCT(BlueprintType)
struct FArtifactData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString Name;                       // 아티팩트 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString Description;                // 아티팩트 설명

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EArtifactType Type;                 // 아티팩트 타입

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    EArtifactSetType SetType;           // 세트 타입

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    //class UTexture2D* Icon;             // 아이콘 이미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FStatBonus StatBonus;               // 개별 스탯 보너스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    bool bIsOpartz = false;             // 오파츠 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    int32 RequiredLevel = 1;            // 필요 레벨

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    int32 Rarity = 1;                   // 희귀도 (1-5)
};

// 세트 보너스 정보 구조체
USTRUCT(BlueprintType)
struct FSetBonusData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
    EArtifactSetType SetType;           // 세트 타입

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
    int32 RequiredPieces = 2;           // 필요한 세트 피스 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
    FStatBonus SetBonus;                // 세트 보너스 스탯

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Bonus")
    FString SetBonusDescription;        // 세트 보너스 설명
};

// 현재 장착된 아티팩트 정보
USTRUCT(BlueprintType)
struct FEquippedArtifact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipped")
    FName ArtifactID;                   // 아티팩트 ID (데이터 테이블의 Row Name)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipped")
    bool bIsEquipped = false;           // 장착 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipped")
    FArtifactData ArtifactData;         // 아티팩트 데이터 (캐시용)
};

UCLASS(BlueprintType)
class PROJECT_SP_API UArtifactDataAsset : public UObject
{
    GENERATED_BODY()

public:
    // FArtifactData를 에디터에서 사용 가능한 형태로 포함
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FArtifactData ArtifactData;
};