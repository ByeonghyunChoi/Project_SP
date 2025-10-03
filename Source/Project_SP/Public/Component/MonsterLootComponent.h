#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Data/ArtifactData.h" // FArtifactData, EArtifactKinds 등 포함
#include "Data/MonsterDropRate.h" // EMonsterGrade, FMonsterDropRate 포함
#include "MonsterLootComponent.generated.h"

USTRUCT(BlueprintType)
struct FMonsterDropInfo : public FTableRowBase
{
    GENERATED_BODY()

    // 드랍할 아이템의 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Info")
    FName ItemID;

    // 드랍 확률 (0.0 ~ 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Info")
    float DropChance;

    // 드랍될 아이템의 개수 (범위)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop Info")
    FInt32Range DropCount;
};

class UArtifactItem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UMonsterLootComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMonsterLootComponent();

    // 몬스터가 처치되었을 때 호출될 함수 (아티팩트 생성만 수행)
    UFUNCTION(BlueprintCallable, Category = "Loot")
    void OnMonsterDefeated(EMonsterGrade MonsterGrade);

    // 언리얼 에디터에서 드랍 테이블을 지정할 수 있습니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    TObjectPtr<class UDataTable> LootTable;

private:
    // 몬스터 등급에 따라 드랍 확률을 가져와 아티팩트 등급을 결정하는 함수
    EArtifactGrade DetermineArtifactGrade(const FMonsterDropRate& DropRates);

    // 아티팩트 종류를 5가지 중 20% 균등 확률로 결정
    EArtifactKinds DetermineArtifactKind(UDataTable* KindWeightsTable);

    // 특수 옵션 8가지 중 하나를 균등 확률로 랜덤 선택
    ESpecialOptionType DetermineSpecialOptionType();

    // DT_ArtifactBaseStats를 사용해 최종 아티팩트 스탯을 계산하고 FArtifactData에 채움
    bool CalculateAndPopulateArtifactData(FArtifactData& OutArtifactData, EArtifactGrade Grade, EArtifactType Type, EArtifactKinds Kind, UDataTable* BaseStatsTable);

    // 데이터 테이블 경로
    const FString DropRatesTablePath = TEXT("/Game/DataTable/DT_MonsterDropRates.DT_MonsterDropRates");
    const FString ArtifactStatsTablePath = TEXT("/Game/DataTable/DT_ArtifactStats.DT_ArtifactStats");
};