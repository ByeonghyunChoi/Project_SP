// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtifactSystemComponent.h"

UArtifactSystemComponent::UArtifactSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UArtifactSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    RecalculateStats();
}


void UArtifactSystemComponent::EquipArtifact(const FArtifactData& Artifact) // 아티팩트 장착
{
    EquippedArtifacts.Add(Artifact.Type, Artifact);
    RecalculateStats();
}

void UArtifactSystemComponent::EquipOrparts(const FOrpartsData& Orparts) // 오파츠 장착
{
    EquippedOrparts = Orparts;
    bHasOrpartsEquipped = true;
    RecalculateStats();
}

void UArtifactSystemComponent::UnequipArtifact(EArtifactType SlotType) // 아티팩트 해제
{
    EquippedArtifacts.Remove(SlotType);
    RecalculateStats();
}

void UArtifactSystemComponent::UnequipOrparts() // 오파츠 해제
{
    EquippedOrparts = FOrpartsData();
    bHasOrpartsEquipped = false;
    RecalculateStats();
}

void UArtifactSystemComponent::UpgradeOrparts(FName MaterialArtifactID) // 오파츠 업그레이드
{
    if (bHasOrpartsEquipped && EquippedOrparts.Level < 10)
    {
        ++EquippedOrparts.Level;
        // 레벨당 능력치 증가 로직은 여기서 구현
        RecalculateStats();
    }
}

FStatBonus UArtifactSystemComponent::GetTotalStatBonus() const
{
    return TotalStats;
}

void UArtifactSystemComponent::RecalculateStats()
{
    TotalStats = FStatBonus();

    TMap<ESetType, int32> SetCounts;

    // 아티팩트 능력치 합산
    for (const auto& Pair : EquippedArtifacts)
    {
        const FArtifactData& Artifact = Pair.Value;
        TotalStats.HP += Artifact.StatBonus.HP;
        TotalStats.Attack += Artifact.StatBonus.Attack;
        TotalStats.Defense += Artifact.StatBonus.Defense;
        TotalStats.Speed += Artifact.StatBonus.Speed;

        if (Artifact.SpecialValue > 0.f)
        {
            TotalStats.SpecialStats.FindOrAdd(Artifact.SpecialStat) += Artifact.SpecialValue;
        }

        SetCounts.FindOrAdd(Artifact.SetType)++;
    }

    // 세트 효과 적용 구현 부분
	// 아티팩트가 일정 점수 이상이 되면 세트효과를 받음 - 아티팩트는 각 등급마다 일정 점수를 가지고 있음
    //
    //
    // 
    // 
    // 
    // 
    // 




    // 오파츠 능력치 합산
    if (bHasOrpartsEquipped)
    {
        TotalStats.HP += EquippedOrparts.BaseStats.HP;
        TotalStats.Attack += EquippedOrparts.BaseStats.Attack;
        TotalStats.Defense += EquippedOrparts.BaseStats.Defense;
        TotalStats.Speed += EquippedOrparts.BaseStats.Speed;

        for (const auto& Spec : EquippedOrparts.SpecialStats)
        {
            TotalStats.SpecialStats.FindOrAdd(Spec.Key) += Spec.Value;
        }
    }

    OnStatsChanged.Broadcast(TotalStats);
}

// 현재 장착된 아티팩트 목록
FArtifactData UArtifactSystemComponent::GetEquippedArtifact(EArtifactType Type) const
{
    if (const FArtifactData* Found = EquippedArtifacts.Find(Type))
    {
        return *Found;
    }

    return FArtifactData(); // 비어있는 기본값 반환
}

// 현재 장착된 오파츠
FOrpartsData UArtifactSystemComponent::GetEquippedOrparts() const
{
    return bHasOrpartsEquipped ? EquippedOrparts : FOrpartsData();
}