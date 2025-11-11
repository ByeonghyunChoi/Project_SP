// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/TimeForceSubsystem.h"
#include "Map/MapManagerSubsystem.h"

void UTimeForceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	StartingTimeForce = 200;

	ResetTimeForce();
}

bool UTimeForceSubsystem::DecreaseTimeForce(int32 AmountToDecrease)
{
	CurrentTimeForce -= AmountToDecrease;
	OnTimeForceChanged.Broadcast(CurrentTimeForce);

	if (CurrentTimeForce <= 0)
	{
		CurrentTimeForce = 0;
		HandleGameOver();
		return false; // 게임 오버됨
	}
	return true;
}

int32 UTimeForceSubsystem::GetCurrentTimeForce() const
{
	return CurrentTimeForce;
}

void UTimeForceSubsystem::ResetTimeForce()
{
	CurrentTimeForce = StartingTimeForce;

	OnTimeForceChanged.Broadcast(CurrentTimeForce);
}

void UTimeForceSubsystem::HandleGameOver()
{
	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		MapManager->ReturnToHub(false); // (패배)
	}
}
