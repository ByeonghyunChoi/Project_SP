// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/RewardBox.h"
#include "Data/RewardData.h"
#include "Character/PlayerCharacter.h"
#include "Component/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ARewardBox::ARewardBox()
{
    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    SetRootComponent(BoxMesh);

    bHasBeenInteracted = false;
}

void ARewardBox::PerformInteraction(APlayerCharacter* Interactor)
{
    if (bHasBeenInteracted)
    {
        return;
    }

    bHasBeenInteracted = true;

    OnRewardInteracted.Broadcast();
    SetActorEnableCollision(false);

    //보상을 주는 로직을 여기에 구현
    RewardToPlayer(Interactor);

    this->Destroy();
}

void ARewardBox::ExecuteInteraction(APlayerCharacter* Interactor)
{
    PerformInteraction(Interactor);
}

void ARewardBox::InitializeReward(UDataTable* InTable, FName InRowName)
{
    RewardInfo = InTable;
	RewardRowName = InRowName;
}

FText ARewardBox::GetInteractText()
{
    return FText::FromString(TEXT("보상 열기[F]"));
}

void ARewardBox::RewardToPlayer(APlayerCharacter* Interactor)
{
    if (!RewardInfo || RewardRowName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("보상 데이터가 설정되지 않았습니다!"));
        return;
    }
    static const FString ContextString(TEXT("Reward Box Context"));
    FRewardData* RewardRow = RewardInfo->FindRow<FRewardData>(RewardRowName, ContextString);

    if (!RewardRow)
    {
        UE_LOG(LogTemp, Error, TEXT("RowName [%s]을 찾을 수 없습니다!"), *RewardRowName.ToString());
        return;
    }

	UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();
	
    if (InventoryComp)
    {
        InventoryComp->GainSand(RewardRow->SandAmount);
        InventoryComp->GainIncompleteEnergy(RewardRow->IncompleteEnergyAmount);
        InventoryComp->GainMoney(RewardRow->MoneyAmount);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("플레이어의 인벤토리 컴포넌트를 찾을 수 없습니다!"));
    }
}



