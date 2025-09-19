#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Data/ArtifactData.h" 
#include "Data/MonsterDropRate.h"
#include "MonsterLootComponent.generated.h"

// 몬스터 드랍 정보 데이터 구조체입니다.
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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UMonsterLootComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMonsterLootComponent();

    // 몬스터가 처치되었을 때 호출될 함수
    UFUNCTION(BlueprintCallable, Category = "Loot")
    void OnMonsterDefeated(EMonsterGrade MonsterGrade);

    // 언리얼 에디터에서 드랍 테이블을 지정할 수 있습니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    TObjectPtr<class UDataTable> LootTable;

private:
	// 몬스터 등급에 따라 드랍 확률을 가져오는 함수
    EArtifactGrade DetermineArtifactGrade(const FMonsterDropRate& DropRates);
    // 등급과 유형에 따라 아티팩트의 스탯을 계산하고 설정합니다.
    void CalculateArtifactStats(class UArtifactItem* Artifact, EArtifactGrade ArtifactGrade, EArtifactType ArtifactType);
};