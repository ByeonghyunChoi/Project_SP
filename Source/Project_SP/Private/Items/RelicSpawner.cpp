// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RelicSpawner.h"
#include "Engine/DataTable.h"
#include "Data/RelicData.h"

URelicSpawner::URelicSpawner()
{

}

TArray<FRelicData> URelicSpawner::GenerateRandomRelicChoices(UDataTable* RelicDataTable, const TArray<FName>& EquippedRelicIDs)
{
	return TArray<FRelicData>();
}

