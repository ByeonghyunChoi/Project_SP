// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/MapBase.h"
#include "Components/SceneComponent.h"
#include "Map/MapNode.h"
#include "Map/PortalActor.h"
#include "Map/RewardBox.h"

// Sets default values
AMapBase::AMapBase()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot")));
	PlayerStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerStartPoint"));
	PlayerStartPoint->SetupAttachment(RootComponent);

	CurrentMapState = EMapState::InProgress;
	CurrentMapType = EMapType::Normal;
}

// Called when the game starts or when spawned
void AMapBase::BeginPlay()
{
	Super::BeginPlay();

	for (APortalActor* Portal : PortalActors)
	{
		if (Portal)
		{
			Portal->SetActorHiddenInGame(false);
			Portal->SetActorEnableCollision(false);
		}
	}

	if (RewardBox)
	{
		RewardBox->SetActorHiddenInGame(true);
		RewardBox->SetActorEnableCollision(false);
	}
	
}

void AMapBase::BeginMapLogic_Implementation()
{
	ActivatePortals();
}

void AMapBase::InitializeNextNodes(const TArray<UMapNode*>& ChildNodes)
{
	NextNodeOptions = ChildNodes;
}

void AMapBase::ActivatePortals()
{
	SetMapState(EMapState::Cleard);
	if (NextNodeOptions.Num() != PortalActors.Num())
	{
		return;
	}

	for (int32 i = 0; i < PortalActors.Num(); ++i)
	{
		APortalActor* Portal = PortalActors[i];
		UMapNode* NodeData = NextNodeOptions[i];

		if (Portal && NodeData)
		{
			Portal->SetActorEnableCollision(true);
			Portal->InitializePortalData(NodeData);
		}
	}
}

void AMapBase::SetMapType(const EMapType& NewMapType)
{
	CurrentMapType = NewMapType;
}

EMapType AMapBase::GetMapType() const
{
	return CurrentMapType;
}

void AMapBase::SetMapState(const EMapState& NewMapState)
{
	CurrentMapState = NewMapState;
}

EMapState AMapBase::GetMapState() const
{
	return CurrentMapState;
}

FVector AMapBase::GetPlayerStartLocation() const
{
	return PlayerStartPoint ? PlayerStartPoint->GetComponentLocation() : GetActorLocation();
}

FRotator AMapBase::GetPlayerStartRotation() const
{
	return PlayerStartPoint ? PlayerStartPoint->GetComponentRotation() : GetActorRotation();
}

