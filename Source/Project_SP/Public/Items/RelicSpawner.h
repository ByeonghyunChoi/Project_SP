// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelicSpawner.generated.h"

struct FRelicData;
class UDataTable;

UCLASS()
class PROJECT_SP_API URelicSpawner : public UObject
{
	GENERATED_BODY()
public:
	URelicSpawner();

	UFUNCTION(BlueprintCallable, Category = "Relic Generation")
	TArray<FRelicData> GenerateRandomRelicChoices(UDataTable* RelicDataTable, const TArray<FName>& EquippedRelicIDs);

protected:
	// 데이터 테이블 참조
	UPROPERTY(EditDefaultsOnly, Category = "Relic Data")
	TObjectPtr<UDataTable> DefaultRelicDataTable;

};