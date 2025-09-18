// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/MonsterLootComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Equipment/InventoryComponent.h"
#include "Equipment/ItemBase.h"
#include "Equipment/ArtifactItem.h"
#include "Data/ArtifactData.h"
#include "Data/MonsterDropRate.h"

const FString DropRatesTablePath = TEXT("/Game/DataTable/DT_MonsterDropRates.DT_MonsterDropRates");


UMonsterLootComponent::UMonsterLootComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 몬스터가 처치되었을 때 호출되는 함수 // 일단은 플레이어와 콜리전 반응으로 추가 예정
void UMonsterLootComponent::OnMonsterDefeated(EMonsterGrade MonsterGrade)
{
    UE_LOG(LogTemp, Log, TEXT("OnMonsterDefeated 함수가 호출되었습니다!"));
    // 1. 드랍 확률 데이터 테이블 로드
    UDataTable* DropRateTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DropRatesTablePath));
    if (!DropRateTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Find MonsterDropTable"));
    }

    if (!LootTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("LootTable is not set on MonsterLootComponent!"));
        return;
    }

    // UEnum 인스턴스를 가져옵니다.
    const UEnum* EnumPtr = StaticEnum<EMonsterGrade>();
    if (!EnumPtr) return;

    // 열거형의 짧은 이름을 FName으로 가져옵니다.
    FName RowName = FName(*EnumPtr->GetNameStringByValue(static_cast<int64>(MonsterGrade)));

    FMonsterDropRate* DropRates = DropRateTable->FindRow<FMonsterDropRate>(RowName, TEXT(""));
    if (!DropRates)
    {
        UE_LOG(LogTemp, Error, TEXT("DropRateTable::FindRow failed for row: %s"), *RowName.ToString());
        return;
    }

    // 2. 아티팩트 등급 결정
    EArtifactGrade ArtifactGrade = DetermineArtifactGrade(*DropRates);
    if (ArtifactGrade == EArtifactGrade::EAG_Normal && FMath::FRand() > DropRates->NormalChance) return; // 드랍되지 않는 경우
    UE_LOG(LogTemp, Log, TEXT("Artifact Grade: %s"), *UEnum::GetValueAsString(ArtifactGrade));

    // 3. 아티팩트 유형 결정 (무작위)
    TArray<EArtifactType> AllTypes;
    AllTypes.Add(EArtifactType::EAT_Crystal);
    AllTypes.Add(EArtifactType::EAT_Oxyge);
    AllTypes.Add(EArtifactType::EAT_Goldberg);
    EArtifactType ArtifactType = AllTypes[FMath::RandRange(0, AllTypes.Num() - 1)];

    // 4. 플레이어 인벤토리 컴포넌트 찾기
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    UInventoryComponent* PlayerInventory = PlayerPawn->FindComponentByClass<UInventoryComponent>();
    if (!PlayerInventory) return;

    // 5. 아티팩트 객체 생성
    FString ItemIDString = UEnum::GetValueAsString(ArtifactGrade) + "_" + UEnum::GetValueAsString(ArtifactType);
    FName ItemID = FName(*ItemIDString);

    UArtifactItem* NewArtifact = NewObject<UArtifactItem>(PlayerInventory);
    if (NewArtifact)
    {
        NewArtifact->InitializeItem(ItemID, 1);
        CalculateArtifactStats(NewArtifact, ArtifactGrade, ArtifactType);
        PlayerInventory->AddItem(ItemID, 1); // 인벤토리에 추가 (ItemID, Count)
        UE_LOG(LogTemp, Log, TEXT("Add New Artifact in Inventory"));
    }
}

// 등급별 드랍 확률을 계산하여 아티팩트 등급을 결정하는 함수
EArtifactGrade UMonsterLootComponent::DetermineArtifactGrade(const FMonsterDropRate& DropRates)
{
    float RandomValue = FMath::FRandRange(0.0f, 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Artifact Grade: %f"), RandomValue);
    // 확률 순서를 높은 등급부터 체크하여 중복 방지
    if (RandomValue <= DropRates.MysticChance) return EArtifactGrade::EAG_Mythic;
    if (RandomValue <= DropRates.LegendaryChance + DropRates.MysticChance) return EArtifactGrade::EAG_Legendary;
    if (RandomValue <= DropRates.UniqueChance + DropRates.LegendaryChance + DropRates.MysticChance) return EArtifactGrade::EAG_Unique;
    if (RandomValue <= DropRates.RareChance + DropRates.LegendaryChance + DropRates.MysticChance + DropRates.UniqueChance) return EArtifactGrade::EAG_Rare;
    if (RandomValue <= DropRates.NormalChance + DropRates.RareChance + DropRates.LegendaryChance + DropRates.MysticChance + DropRates.UniqueChance) return EArtifactGrade::EAG_Normal;

	return EArtifactGrade::EAG_Normal; // 기본값 // 일단 나중에 드랍 안되게 처리
}

// 등급과 유형에 따라 아티팩트의 스탯을 계산하고 설정하는 함수
void UMonsterLootComponent::CalculateArtifactStats(UArtifactItem* Artifact, EArtifactGrade ArtifactGrade, EArtifactType ArtifactType)
{
    if (!Artifact) return;

    TMap<EArtifactGrade, float> GradeMultipliers;
    GradeMultipliers.Add(EArtifactGrade::EAG_Normal, 1.0f);
    GradeMultipliers.Add(EArtifactGrade::EAG_Rare, 1.3f);
    GradeMultipliers.Add(EArtifactGrade::EAG_Unique, 1.5f);
    GradeMultipliers.Add(EArtifactGrade::EAG_Legendary, 1.5f * 1.5f);
    GradeMultipliers.Add(EArtifactGrade::EAG_Mythic, 1.5f * 1.5f * 1.5f);
    float Multiplier = GradeMultipliers.Contains(ArtifactGrade) ? GradeMultipliers[ArtifactGrade] : 1.0f;

    // 특수 옵션 스탯 계산 (첨부된 사진 참조)
    if (Artifact->ArtifactData.SpecialOptionType != ESpecialOptionType::ESOT_None)
    {
        float BaseValue = 0.0f;
        float GradeIncrement = 0.0f;

        switch (Artifact->ArtifactData.SpecialOptionType)
        {
        case ESpecialOptionType::ESOT_CritChance: BaseValue = 4.0f; GradeIncrement = 4.0f; break;
        case ESpecialOptionType::ESOT_CritDamage: BaseValue = 10.0f; GradeIncrement = 6.0f; break;
        //case ESpecialOptionType::ESOT_StatusAccuracy: BaseValue = 5.0f; GradeIncrement = 5.0f; break;
		//case ESpecialOptionType::ESOT_DefensePenetration: BaseValue = 4.0f; GradeIncrement = 4.0f; break; // 방어 관통 옵션은 제외
        case ESpecialOptionType::ESOT_Attack: BaseValue = 10.0f; GradeIncrement = 5.0f; break;
        case ESpecialOptionType::ESOT_Defense: BaseValue = 10.0f; GradeIncrement = 5.0f; break;
        case ESpecialOptionType::ESOT_Health: BaseValue = 10.0f; GradeIncrement = 5.0f; break;
        case ESpecialOptionType::ESOT_Speed: BaseValue = 10.0f; GradeIncrement = 6.0f; break;
        default: break;
        }

        float FinalValue = BaseValue + (GradeIncrement * (static_cast<int32>(ArtifactGrade) - static_cast<int32>(EArtifactGrade::EAG_Normal)));
        Artifact->ArtifactData.SpecialOptionValue = FinalValue;
    }
}

