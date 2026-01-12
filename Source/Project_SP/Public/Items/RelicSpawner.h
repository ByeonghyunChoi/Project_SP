//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "UObject/NoExportTypes.h"
//#include "Data/RelicData.h"
//#include "RelicSpawner.generated.h"
//
//class UDataTable;
//class URelicBase;
//
//UCLASS()
//class PROJECT_SP_API URelicSpawner : public UObject
//{
//	GENERATED_BODY()
//
//public:
//	URelicSpawner();
//
//	UFUNCTION(BlueprintCallable, Category = "Relic Generation")
//	TArray<FRelicData> GenerateRandomRelicChoices(UDataTable* RelicDataTable, const TArray<TSubclassOf<URelicBase>>& EquippedRelicClasses);
//
//protected:
//
//};