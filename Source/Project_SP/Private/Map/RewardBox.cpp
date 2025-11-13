// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/RewardBox.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ARewardBox::ARewardBox()
{
    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    SetRootComponent(BoxMesh);

    bHasBeenInteracted = false;
}

void ARewardBox::PerformInteraction()
{
    if (bHasBeenInteracted)
    {
        return;
    }

    bHasBeenInteracted = true;

    OnRewardInteracted.Broadcast();
    SetActorEnableCollision(false);

    //보상을 주는 로직을 여기에 구현

    this->Destroy();
}

void ARewardBox::ExecuteInteraction(APlayerCharacter* Interactor)
{
    PerformInteraction();
}

FText ARewardBox::GetInteractText()
{
    return FText::FromString(TEXT("보상 열기[F]"));
}



