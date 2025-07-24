#include "ArtifactGeneratorComponent.h"

UArtifactGeneratorComponent::UArtifactGeneratorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 기본 등급 확률 (데이터 기반으로 바꿔도 됨)
    ArtifactRarityRates.Add(ERarity::Normal, 0.4f);
    ArtifactRarityRates.Add(ERarity::Rare, 0.3f);
    ArtifactRarityRates.Add(ERarity::Unique, 0.15f);
    ArtifactRarityRates.Add(ERarity::Legendary, 0.1f);
    ArtifactRarityRates.Add(ERarity::Mystic, 0.05f);

    OrpartsRarityRates = ArtifactRarityRates;

    // 특수옵션 풀 초기화
    SpecialStatPool = {
        ESpecialStatType::CriticalChance,
        ESpecialStatType::CriticalDamage,
        ESpecialStatType::ArmorPenetration,
        ESpecialStatType::AttackPercent,
        ESpecialStatType::DefensePercent,
        ESpecialStatType::HPPercent,
        ESpecialStatType::SpeedPercent
    };
}

void UArtifactGeneratorComponent::BeginPlay()
{
    Super::BeginPlay();

    // 에디터에서 입력한 DropSettings → 실제 캐시용 Map 변환
    CachedDropRateTable.Empty();

    for (const FMonsterDropSetting& Setting : DropSettings)
    {
        FDropRateTable Table;
        Table.OrpartsDropChance = Setting.OrpartsDropChance;

        for (const FRarityRate& Rate : Setting.ArtifactDropRates)
        {
            Table.ArtifactDropRates.Add(Rate.Rarity, Rate.Rate);
        }

        CachedDropRateTable.Add(Setting.MonsterGrade, Table);

        Table.ArtifactDropChance = Setting.ArtifactDropChance;
    }
}

bool UArtifactGeneratorComponent::ShouldDropArtifact(EMonsterGrade Grade, ERarity& OutRarity)
{
    if (!CachedDropRateTable.Contains(Grade)) return false;

    const FDropRateTable& DropTable = CachedDropRateTable[Grade];

	// 드랍 확률 계산
    if (FMath::FRand() > DropTable.OrpartsDropChance)
    {
        return false; // 실패
	}

    float Rand = FMath::FRand();
    float Accum = 0.f;

    for (const auto& Pair : DropTable.ArtifactDropRates)
    {
        Accum += Pair.Value;
        if (Rand <= Accum)
        {
            OutRarity = Pair.Key;
            return true;
        }
    }

    return false;
}

bool UArtifactGeneratorComponent::ShouldDropOrparts(EMonsterGrade Grade)
{
    if (!CachedDropRateTable.Contains(Grade)) return false;

    const FDropRateTable& DropTable = CachedDropRateTable[Grade];
    return FMath::FRand() <= DropTable.OrpartsDropChance;
}

FArtifactData UArtifactGeneratorComponent::GenerateRandomArtifact_RandomTypeSet()
{
    // 랜덤 타입의 마지막 값을 값으로 지정(Special)
	int32 TypeIndex = FMath::RandRange(0, static_cast<int32>(EArtifactType::Special));
	EArtifactType RandomType = static_cast<EArtifactType>(TypeIndex);

	// 랜덤 세트 타입의 마지막 값을 값으로 지정(Gold)
	int32 SetIndex = FMath::RandRange(0, static_cast<int32>(ESetType::Gold));
	ESetType RandomSet = static_cast<ESetType>(SetIndex);

	// 아티팩트 생성
    return GenerateRandomArtifact(RandomType, RandomSet);
}

FArtifactData UArtifactGeneratorComponent::GenerateRandomArtifact(EArtifactType Type, ESetType SetType) // 아티팩트 생성
{
    ERarity Rarity = GetRandomRarity(ArtifactRarityRates);
    ESpecialStatType Special = GetRandomSpecialStat();
    float SpecialValue = GetRandomSpecialValue(Rarity);

    FArtifactData NewArtifact;
    NewArtifact.Type = Type;
    NewArtifact.SetType = SetType;
    NewArtifact.Rarity = Rarity;
    NewArtifact.SpecialStat = Special;
    NewArtifact.SpecialValue = SpecialValue;
    NewArtifact.ID = FName(*FString::Printf(TEXT("Artifact_%d"), FMath::Rand()));

    NewArtifact.StatBonus = GetBaseStatsFromType(Type, Rarity);

    return NewArtifact;
}

FOrpartsData UArtifactGeneratorComponent::GenerateRandomOrparts(ESetType SetType)
{
    ERarity Rarity = GetRandomRarity(OrpartsRarityRates);
    ESpecialStatType StatA = GetRandomSpecialStat();
    ESpecialStatType StatB = GetRandomSpecialStat();

    while (StatA == StatB)
    {
        StatB = GetRandomSpecialStat();
    }

    FOrpartsData NewOrparts;
    NewOrparts.ID = FName(*FString::Printf(TEXT("Orparts_%d"), FMath::Rand()));
    NewOrparts.Rarity = Rarity;
    NewOrparts.SetType = SetType;
    NewOrparts.Level = 1;

    NewOrparts.BaseStats.HP = 100.f * (1 + static_cast<int>(Rarity) * 0.3f);
    NewOrparts.BaseStats.Attack = 50.f * (1 + static_cast<int>(Rarity) * 0.3f);
    NewOrparts.BaseStats.Defense = 50.f * (1 + static_cast<int>(Rarity) * 0.3f);

    NewOrparts.SpecialStats.Add(StatA, GetRandomSpecialValue(Rarity));
    NewOrparts.SpecialStats.Add(StatB, GetRandomSpecialValue(Rarity));

    return NewOrparts;
}

ERarity UArtifactGeneratorComponent::GetRandomRarity(const TMap<ERarity, float>& RateTable) const
{
    float Rand = FMath::FRand();
    float Accum = 0.f;

    for (const auto& Pair : RateTable)
    {
        Accum += Pair.Value;
        if (Rand <= Accum)
        {
            return Pair.Key;
        }
    }

    return ERarity::Normal; // fallback
}

ESpecialStatType UArtifactGeneratorComponent::GetRandomSpecialStat() const
{
    int32 Index = FMath::RandRange(0, SpecialStatPool.Num() - 1);
    return SpecialStatPool[Index];
}

float UArtifactGeneratorComponent::GetRandomSpecialValue(ERarity Rarity) const
{
    float Base = 5.0f;
    float Multiplier = 1.0f + static_cast<int>(Rarity) * 0.5f;
    return Base * Multiplier;
}

FStatBonus UArtifactGeneratorComponent::GetBaseStatsFromType(EArtifactType Type, ERarity Rarity) const
{
    float Scale = 10.f + static_cast<int>(Rarity) * 5.f;
    FStatBonus Bonus;

    switch (Type)
    {
    case EArtifactType::HPUp: Bonus.HP = Scale * 10.f; break;
    case EArtifactType::ATKUp: Bonus.Attack = Scale; break;
    case EArtifactType::DEFUp: Bonus.Defense = Scale; break;
    case EArtifactType::SPDUp: Bonus.Speed = Scale * 0.1f; break;
    default: break;
    }

    return Bonus;
}
