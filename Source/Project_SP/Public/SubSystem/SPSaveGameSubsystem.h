// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SPDataStructs.h"
#include "SPSaveGameSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SavePlayerStats(APawn* PlayerPawn);

	// 불러오기: Subsystem -> Player
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void LoadPlayerStats(APawn* PlayerPawn);

	// 초기화
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void ResetSaveData();

	// 데이터 확인용 Getter
	const FPlayerPersistentData& GetCurrentSaveData() const { return SaveData; }

private:
	UPROPERTY()
	FPlayerPersistentData SaveData;
};
