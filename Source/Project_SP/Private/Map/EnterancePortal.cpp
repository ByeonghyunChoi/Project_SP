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
    UGameInstance* GI = GetGameInstance();
    if (GI)
    {
        UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>();
        if (MapManager)
        {
            UE_LOG(LogTemp, Log, TEXT("Starting New Run from Lobby..."));

            // ★ 여기서 게임 시작!
            MapManager->StartNewRun();
        }
    }
}

FText AEnterancePortal::GetInteractText() const
{
	return FText();
}

