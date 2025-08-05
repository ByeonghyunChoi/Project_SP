#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArtifactTypes.h"
#include "ArtifactData.h"
#include "OrpartsData.h"
#include "ArtifactGeneratorComponent.generated.h"

// 등급별 확률 정보 구조체
USTRUCT(BlueprintType)
struct FRarityRate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Rate;
};

// 몬스터 등급별 드랍 정보
USTRUCT(BlueprintType)
struct FMonsterDropSetting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMonsterGrade MonsterGrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRarityRate> ArtifactDropRates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArtifactDropChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OrpartsDropChance;

};

// 실제 드랍 판정을 위한 내부 구조체
struct FDropRateTable
{
	float ArtifactDropChance = 0.f;
    TMap<ERarity, float> ArtifactDropRates;
    float OrpartsDropChance = 0.f;
};

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UArtifactGeneratorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArtifactGeneratorComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 아티팩트 타입과 세트타입 정하기
	UFUNCTION(BlueprintCallable, Category = "Artifact Generator")
	FArtifactData GenerateRandomArtifact_RandomTypeSet();

    // 아티팩트 생성
    UFUNCTION(BlueprintCallable, Category = "Artifact Generator")
    FArtifactData GenerateRandomArtifact(EArtifactType Type, ESetType SetType);

    // 오파츠 생성
    UFUNCTION(BlueprintCallable, Category = "Artifact Generator")
    FOrpartsData GenerateRandomOrparts(ESetType SetType);

    // 드랍 가능 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Artifact Generator")
    bool ShouldDropArtifact(EMonsterGrade Grade, ERarity& OutRarity);

    UFUNCTION(BlueprintCallable, Category = "Artifact Generator")
    bool ShouldDropOrparts(EMonsterGrade Grade);

    // 에디터에서 설정 가능한 드랍 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
    TArray<FMonsterDropSetting> DropSettings;

    // 아티팩트 기본 생성 확률
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Probability")
    TMap<ERarity, float> ArtifactRarityRates;

    // 특수옵션 후보 리스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Stats")
    TArray<ESpecialStatType> SpecialStatPool;

protected:
    // 내부 캐싱용: 몬스터 등급 → 드랍 확률 테이블
    TMap<EMonsterGrade, FDropRateTable> CachedDropRateTable;

    ERarity GetRandomRarity(const TMap<ERarity, float>& RateTable) const;

    ESpecialStatType GetRandomSpecialStat() const;

    float GetRandomSpecialValue(ERarity Rarity) const;

    FStatBonus GetBaseStatsFromType(EArtifactType Type, ERarity Rarity) const;
};
