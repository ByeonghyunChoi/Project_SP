// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/EnterancePortal.h"
#include "Map/MapManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnterancePortal::AEnterancePortal()
{
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
}

void AEnterancePortal::ExecuteInteraction(AActor* Interactor)
{
   IInteractableInterface::Execute_PlayInteractSound(this);
}

FText AEnterancePortal::GetInteractText() const
{
	return FText::FromString(TEXT("게임 시작"));
}

void AEnterancePortal::PlayInteractSound_Implementation()
{
}

void AEnterancePortal::ExecutePortalTransition()
{
    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>();
        if (MapManager)
        {
            UE_LOG(LogTemp, Log, TEXT("Starting New Run from Lobby..."));
            IInteractableInterface::Execute_PlayInteractSound(this);
            // ★ 여기서 게임 시작!
            MapManager->StartNewRun();
        }
    }
}

