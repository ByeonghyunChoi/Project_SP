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


FName AMapBase::GetRewardRowNameByMapType() const
{
	switch (CurrentMapType)
    {
    case EMapType::NormalBattle:
        return FName("Normal"); // 일반 전투 보상

    case EMapType::StrongEnemyBattle:
        return FName("Epic");  // 강적 전투 보상

    case EMapType::BossBattle:
        return FName("Boss");   // 보스 전투 보상

    case EMapType::Jester:
        return FName("Epic");   // 이벤트 맵 보상

    default:
        return FName("Normal");
    }
}

void AMapBase::BeginMapLogic_Implementation()
{
	ActivatePortals();
}

void AMapBase::OnCombatFinished_Implementation(bool bPlayerWon)
{
	UE_LOG(LogTemp, Log, TEXT("AMapBase::OnCombatFinished - PlayerWon: %s"), bPlayerWon ? TEXT("True") : TEXT("False"));
	if (bPlayerWon)
	{
		// 보상 상자 설정
		if (RewardBox)
		{
			// ... 보상 설정 로직 ...
			FName TargetLootGroup = GetRewardRowNameByMapType();
			RewardBox->InitializeReward(RewardDataTable, TargetLootGroup, RelicDataTable);
		}
	}
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

