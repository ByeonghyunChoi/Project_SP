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
	CurrentMapType = EMapType::NormalBattle;
}


void AMapBase::BeginMapLogic_Implementation()
{
	ActivatePortals();
}

void AMapBase::OnCombatFinished_Implementation(bool bPlayerWon)
{
	UE_LOG(LogTemp, Log, TEXT("AMapBase::OnCombatFinished - PlayerWon: %s"), bPlayerWon ? TEXT("True") : TEXT("False"));
}

void AMapBase::InitializeNextNodes(const TArray<UMapNode*>& ChildNodes)
{
	NextNodeOptions = ChildNodes;
}

void AMapBase::ClearMapElements()
{
	for (APortalActor* Portal : PortalActors)
	{
		if (Portal)
		{
			Portal->Destroy();
		}
	}
	PortalActors.Empty();
}

void AMapBase::ActivatePortals()
{
	SetMapState(EMapState::Cleard);

	int32 NumToActivate = FMath::Min(NextNodeOptions.Num(), PortalActors.Num());
	
	for (int32 i = 0; i < NumToActivate; ++i)
	{
		APortalActor* Portal = PortalActors[i];
		UMapNode* NodeData = NextNodeOptions[i];

		if (Portal && NodeData)
		{
			Portal->SetActorEnableCollision(true);
			Portal->InitializePortalData(NodeData);
			Portal->OnPortalStateChanged(true);
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

