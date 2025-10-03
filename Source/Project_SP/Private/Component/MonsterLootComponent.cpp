

#include "Component/MonsterLootComponent.h"
#include "Kismet/GameplayStatics.h"
// #include "Component/InventoryComponent.h" // 인벤토리 관련 include 제거
#include "Items/ArtifactItem.h"
#include "Data/ArtifactData.h"

UMonsterLootComponent::UMonsterLootComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// 몬스터가 처치되었을 때 호출되는 함수
void UMonsterLootComponent::OnMonsterDefeated(EMonsterGrade MonsterGrade)
{
    // 1. 필요한 데이터 테이블 로드
    UDataTable* DropRateTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DropRatesTablePath));
    UDataTable* BaseStatsTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ArtifactStatsTablePath));

    if (!DropRateTable || !BaseStatsTable)
    {
        UE_LOG(LogTemp, Error, TEXT("아티팩트 생성에 필요한 데이터 테이블(DropRate/BaseStats)을 로드할 수 없습니다."));
        return;
    }

    // 몬스터 등급에 맞는 드랍 확률 행 찾기
    const UEnum* MonsterEnum = StaticEnum<EMonsterGrade>();
    if (!MonsterEnum) return;

    FString EnumString = MonsterEnum->GetNameStringByValue(static_cast<int64>(MonsterGrade));
    FString CleanRowName;
    if (!EnumString.Split(TEXT("_"), nullptr, &CleanRowName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        // Split 실패 시 (접두사가 없거나 형식이 다를 경우) 전체 이름을 사용합니다. 만약을 대비해서 열거형에서 접두사를 사용하지 않았을 경우를 대비.
        CleanRowName = EnumString;
    }

    FName MonsterRowName = FName(*CleanRowName); // 예: "EMG_Epic" -> "Epic"
    FMonsterDropRate* DropRates = DropRateTable->FindRow<FMonsterDropRate>(MonsterRowName, TEXT(""));

    if (!DropRates)
    {
        UE_LOG(LogTemp, Error, TEXT("DropRateTable에서 몬스터 등급 행을 찾을 수 없습니다: %s"), *MonsterRowName.ToString());
        return;
    }

    // 2. 아티팩트 등급 결정 (몬스터 등급에 따른 확률 적용)
    EArtifactGrade ArtifactGrade = DetermineArtifactGrade(*DropRates);

    // 3. 아티팩트 유형 결정 (무작위)
    TArray<EArtifactType> AllTypes;
    AllTypes.Add(EArtifactType::EAT_Crystal);
    AllTypes.Add(EArtifactType::EAT_Oxyge);
    AllTypes.Add(EArtifactType::EAT_Goldberg);
    EArtifactType ArtifactType = AllTypes[FMath::RandRange(0, AllTypes.Num() - 1)];

    // 4. 아티팩트 종류 결정 (5가지 중 20% 균등 확률 적용)
    EArtifactKinds ArtifactKind = DetermineArtifactKind(nullptr);

    // 5. 아티팩트 데이터 조합 및 스탯 계산
    FArtifactData NewArtifactData;
    if (CalculateAndPopulateArtifactData(NewArtifactData, ArtifactGrade, ArtifactType, ArtifactKind, BaseStatsTable))
    {
        // 6. 아티팩트 객체 생성 (인벤토리에 넣지 않으므로 임시 객체로 생성)
        UArtifactItem* NewArtifact = NewObject<UArtifactItem>(this);
        if (NewArtifact)
        {
            // ArtifactItem에 최종 데이터 설정 및 ItemBase 초기화
            NewArtifact->InitializeArtifactData(NewArtifactData);

            // 7. 인벤토리 추가 로직은 제외하고 생성된 아티팩트 정보를 로그로 출력하여 검증
            UE_LOG(LogTemp, Warning, TEXT("=== ARTIFACT CREATED (NO INVENTORY ADD) ==="));
            UE_LOG(LogTemp, Log, TEXT("  - 등급: %s, 유형: %s, 종류: %s"),
                *UEnum::GetValueAsString(NewArtifactData.ArtifactGrade),
                *UEnum::GetValueAsString(NewArtifactData.ArtifactType),
                *UEnum::GetValueAsString(NewArtifactData.ArtifactKinds));

            if (NewArtifactData.ArtifactKinds == EArtifactKinds::EAK_Special)
            {
                UE_LOG(LogTemp, Log, TEXT("  - 특수 옵션: %s, 최종 값: %.2f"),
                    *UEnum::GetValueAsString(NewArtifactData.SpecialOptionType), NewArtifactData.SpecialOptionValue);
            }
            else
            {
                // 생성된 일반 스탯 값 중 0이 아닌 값만 로그로 출력 (예: HealthKind이면 FinalHealth만 0이 아님)
                if (NewArtifactData.FinalHealth > 0) UE_LOG(LogTemp, Log, TEXT("  - 최종 체력: %.2f"), NewArtifactData.FinalHealth);
                if (NewArtifactData.FinalAttack > 0) UE_LOG(LogTemp, Log, TEXT("  - 최종 공격력: %.2f"), NewArtifactData.FinalAttack);
                if (NewArtifactData.FinalDefense > 0) UE_LOG(LogTemp, Log, TEXT("  - 최종 방어력: %.2f"), NewArtifactData.FinalDefense);
                if (NewArtifactData.FinalSpeed > 0) UE_LOG(LogTemp, Log, TEXT("  - 최종 속도: %.2f"), NewArtifactData.FinalSpeed);
            }

            // 임시 객체이므로 다음 틱에서 파괴되도록 예약 (Cleanup)
            NewArtifact->MarkAsGarbage();
        }
    }
}

// 등급별 드랍 확률을 계산하여 아티팩트 등급을 결정하는 함수
EArtifactGrade UMonsterLootComponent::DetermineArtifactGrade(const FMonsterDropRate& DropRates)
{
    float RandomValue = FMath::FRandRange(0.0f, 100.0f);
    float CurrentChance = 0.0f;

    // 높은 등급부터 체크
    CurrentChance += DropRates.MysticChance;
    if (RandomValue <= CurrentChance) return EArtifactGrade::EAG_Mythic;

    CurrentChance += DropRates.LegendaryChance;
    if (RandomValue <= CurrentChance) return EArtifactGrade::EAG_Legendary;

    CurrentChance += DropRates.UniqueChance;
    if (RandomValue <= CurrentChance) return EArtifactGrade::EAG_Unique;

    CurrentChance += DropRates.RareChance;
    if (RandomValue <= CurrentChance) return EArtifactGrade::EAG_Rare;

    CurrentChance += DropRates.NormalChance;
    if (RandomValue <= CurrentChance) return EArtifactGrade::EAG_Normal;

    return EArtifactGrade::EAG_Normal;
}

// 아티팩트 종류를 5가지 중 20% 균등 확률로 결정하는 함수
EArtifactKinds UMonsterLootComponent::DetermineArtifactKind(UDataTable* KindWeightsTable)
{
    const UEnum* EnumPtr = StaticEnum<EArtifactKinds>();
    if (!EnumPtr) return EArtifactKinds::EAK_Health;

    // EAK_Health부터 EAK_Special까지 총 5가지 종류가 균등하게 나옴
    int32 NumKinds = EnumPtr->NumEnums() - 1; // _MAX 값은 제외
    int32 RandomIndex = FMath::RandRange(0, NumKinds - 1);

    return (EArtifactKinds)EnumPtr->GetValueByIndex(RandomIndex);
}

// Special Kind일 경우, 8가지 특수 옵션 중 하나를 균등 확률로 랜덤 선택
ESpecialOptionType UMonsterLootComponent::DetermineSpecialOptionType()
{
    const UEnum* EnumPtr = StaticEnum<ESpecialOptionType>();
    if (!EnumPtr) return ESpecialOptionType::ESOT_None;

    // ESOT_None(0)은 제외하고, ESOT_CritChance(1)부터 ESOT_Speed까지 총 8가지 옵션이 균등하게 나옴
    int32 MinIndex = 1;
    int32 MaxIndex = EnumPtr->NumEnums() - 2;
    int32 RandomIndex = FMath::RandRange(MinIndex, MaxIndex);

    return (ESpecialOptionType)EnumPtr->GetValueByIndex(RandomIndex);
}

// 최종 아티팩트 스탯을 계산하여 FArtifactData에 채우는 함수
bool UMonsterLootComponent::CalculateAndPopulateArtifactData(FArtifactData& OutArtifactData, EArtifactGrade Grade, EArtifactType Type, EArtifactKinds Kind, UDataTable* BaseStatsTable)
{
    // 1. 아티팩트 기본 정보 설정
    OutArtifactData.ArtifactGrade = Grade;
    OutArtifactData.ArtifactType = Type;
    OutArtifactData.ArtifactKinds = Kind;

    ESpecialOptionType SelectedSpecialOption = ESpecialOptionType::ESOT_None;
    FString SpecialRowName;

    if (Kind == EArtifactKinds::EAK_Special)
    {
        SelectedSpecialOption = DetermineSpecialOptionType();
    }
    FArtifactStats* BaseStats = nullptr;

    // 1. Kind에 해당하는 RowName 준비
    FString KindEnumString = UEnum::GetValueAsString(Kind); // EAK_Health
    FString BaseRowName;
    KindEnumString.Split(TEXT("_"), nullptr, &BaseRowName, ESearchCase::IgnoreCase, ESearchDir::FromEnd); // "Health"

    if (Kind != EArtifactKinds::EAK_Special) // 일반 아티팩트
    {
        // 일반 아티팩트 행 이름은 'Health', 'Attack' 등 (BaseRowName과 일치)
        BaseStats = BaseStatsTable->FindRow<FArtifactStats>(FName(*BaseRowName), TEXT("FindBaseStatsDirect"));
    }
    else // 특수 아티팩트
    {
        // 특수 아티팩트 행 이름은 'Special_CritChance', 'Special_Health' 등 (RowName 전체 순회 필요)
        SpecialRowName = TEXT("Special_") + UEnum::GetValueAsString(SelectedSpecialOption);
        FString CleanSpecialRowName;

        if (SpecialRowName.Split(TEXT("ESOT_"), nullptr, &CleanSpecialRowName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
        {
            SpecialRowName = CleanSpecialRowName;
        }

        // RowName을 기반으로 바로 조회 (특수 옵션도 직접 조회)
        BaseStats = BaseStatsTable->FindRow<FArtifactStats>(FName(*SpecialRowName), TEXT("FindSpecialStatsDirect"));
    }
    

    if (!BaseStats)
    {
        UE_LOG(LogTemp, Error, TEXT("DT_ArtifactBaseStats에서 필요한 스탯 행을 찾을 수 없습니다! Kind: %s, SpecialType: %s (RowName 시도: %s)"),
            *UEnum::GetValueAsString(Kind),
            *UEnum::GetValueAsString(SelectedSpecialOption),
            Kind != EArtifactKinds::EAK_Special ? *BaseRowName : *SpecialRowName);
        return false;
    }

    // 2. 등급에 따른 배율 결정
    float Multiplier = 1.0f;
    switch (Grade)
    {
    case EArtifactGrade::EAG_Rare: Multiplier = BaseStats->RareMultiplier; break;
    case EArtifactGrade::EAG_Unique: Multiplier = BaseStats->UniqueMultiplier; break;
    case EArtifactGrade::EAG_Legendary: Multiplier = BaseStats->LegendaryMultiplier; break;
    case EArtifactGrade::EAG_Mythic: Multiplier = BaseStats->MythicMultiplier; break;
    default: break;
    }

    // 3. 최종 스탯 계산 및 적용 (결과를 0으로 초기화 후 시작)
    OutArtifactData.FinalAttack = 0.0f;
    OutArtifactData.FinalDefense = 0.0f;
    OutArtifactData.FinalHealth = 0.0f;
    OutArtifactData.FinalSpeed = 0.0f;
    OutArtifactData.SpecialOptionType = ESpecialOptionType::ESOT_None;
    OutArtifactData.SpecialOptionValue = 0.0f;

    if (Kind != EArtifactKinds::EAK_Special)
    {
        // 일반 능력치 (Health, Attack, Defense, Speed) 처리
        float FinalBaseValue = BaseStats->BaseValue * Multiplier;

        if (Kind == EArtifactKinds::EAK_Health)
        {
            OutArtifactData.FinalHealth = FinalBaseValue;
        }
        else if (Kind == EArtifactKinds::EAK_Attack)
        {
            OutArtifactData.FinalAttack = FinalBaseValue;
        }
        else if (Kind == EArtifactKinds::EAK_Defense)
        {
            OutArtifactData.FinalDefense = FinalBaseValue;
        }
        else if (Kind == EArtifactKinds::EAK_Speed)
        {
            OutArtifactData.FinalSpeed = FinalBaseValue;
        }
    }
    else // 4. 특수 아티팩트 처리
    {
        int32 GradeIndex = static_cast<int32>(Grade) - static_cast<int32>(EArtifactGrade::EAG_Normal); // Normal=0, Rare=1, ...

        OutArtifactData.SpecialOptionType = BaseStats->SpecialOptionType;
        // 최종 값 = 기본값 + (등급 증가치 * 등급 인덱스)
        OutArtifactData.SpecialOptionValue = BaseStats->BaseSpecialValue * Multiplier;
    }

    return true;
}
