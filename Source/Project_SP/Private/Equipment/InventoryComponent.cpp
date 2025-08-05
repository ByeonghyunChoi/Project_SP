// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/InventoryComponent.h"
#include "Equipment/ArtifactSystemComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    ArtifactSystem = GetOwner()->FindComponentByClass<UArtifactSystemComponent>();
    if (!ArtifactSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ArtifactSystemComponent not found on owner!"));
    }
}

// 휙득
void UInventoryComponent::AddArtifact(const FArtifactData& NewArtifact)
{
    ArtifactInventory.Add(NewArtifact);
}

void UInventoryComponent::AddOrparts(const FOrpartsData& NewOrparts)
{
    OrpartsInventory.Add(NewOrparts);
}


// 장착
bool UInventoryComponent::EquipArtifact(FName ArtifactID)
{
    if (!ArtifactSystem) return false;

    int32 Index = ArtifactInventory.IndexOfByPredicate([&](const FArtifactData& A) {
        return A.ID == ArtifactID;
    });

	if (Index != INDEX_NONE) // 아티팩트가 인벤토리에 존재하는지 확인
    {
        FArtifactData ArtifactToEquip = ArtifactInventory[Index];
        ArtifactInventory.RemoveAt(Index);

        // 기존 장착된 아티팩트를 인벤토리에 다시 넣기
        FArtifactData OldEquipped = ArtifactSystem->GetEquippedArtifact(ArtifactToEquip.Type);
        if (!OldEquipped.ID.IsNone())
        {
            AddArtifact(OldEquipped);
        }

        ArtifactSystem->EquipArtifact(ArtifactToEquip);
        return true;
    }

    return false;
}

bool UInventoryComponent::EquipOrparts(FName OrpartsID)
{
    if (!ArtifactSystem) return false;

    int32 Index = OrpartsInventory.IndexOfByPredicate([&](const FOrpartsData& O) {
        return O.ID == OrpartsID;
    });

	if (Index != INDEX_NONE) // 오파츠가 인벤토리에 존재하는지 확인
    {
        FOrpartsData OrpartsToEquip = OrpartsInventory[Index];
        OrpartsInventory.RemoveAt(Index);

        // 기존 장착된 오파츠를 인벤토리에 다시 넣기
        FOrpartsData OldOrparts = ArtifactSystem->GetEquippedOrparts();
        if (!OldOrparts.ID.IsNone())
        {
            AddOrparts(OldOrparts);
        }

        ArtifactSystem->EquipOrparts(OrpartsToEquip);
        return true;
    }

    return false;
}

// 해제
void UInventoryComponent::UnequipArtifact(EArtifactType Type)
{
    if (!ArtifactSystem) return;

    FArtifactData Equipped = ArtifactSystem->GetEquippedArtifact(Type);
    if (!Equipped.ID.IsNone())
    {
        AddArtifact(Equipped);
    }

    ArtifactSystem->UnequipArtifact(Type);
}

void UInventoryComponent::UnequipOrparts()
{
    if (!ArtifactSystem) return;

    FOrpartsData Equipped = ArtifactSystem->GetEquippedOrparts();
    if (!Equipped.ID.IsNone())
    {
        AddOrparts(Equipped);
    }

    ArtifactSystem->UnequipOrparts();
}

// 조회
const FArtifactData* UInventoryComponent::FindArtifactByID(FName ArtifactID) const
{
    return ArtifactInventory.FindByPredicate([&](const FArtifactData& Artifact)
        {
            return Artifact.ID == ArtifactID;
        });
}

const FOrpartsData* UInventoryComponent::FindOrpartsByID(FName OrpartsID) const
{
    return OrpartsInventory.FindByPredicate([&](const FOrpartsData& Orparts)
        {
            return Orparts.ID == OrpartsID;
        });
}