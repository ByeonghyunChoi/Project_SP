// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/MapBase.h"

// Sets default values
AMapBase::AMapBase()
{
 	
}

// Called when the game starts or when spawned
void AMapBase::BeginPlay()
{
	Super::BeginPlay();
	
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

