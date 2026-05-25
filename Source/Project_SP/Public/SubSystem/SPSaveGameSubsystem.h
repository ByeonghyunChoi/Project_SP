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
	// 런 데이터 제어
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Run")
	void CacheRunDataFromPlayer(APawn* PlayerPawn);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Run")
	void RestoreRunDataToPlayer(APawn* PlayerPawn);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Run")
	void ResetRunData(); // 사망, 보스 클리어 시 호출

	// 영구 데이터 제어
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Perm")
	void CachePermDataFromPlayer(APawn* PlayerPawn);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Perm")
	void RestorePermDataToPlayer(APawn* PlayerPawn);

	//데이터 초기화(새로 하기할 때 호출)
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Core")
	void ResetAllData();

	//Getter
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Data")
	const FPlayerRunData& GetRunData() const { return RunData; }

	UFUNCTION(BlueprintPure, Category = "SaveSystem|Data")
	const FPlayerMetaProgressionData& GetPermData() const { return PermData; }

public:
	//세이브 파일 관리

	// 런 데이터 파일 관리
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Disk")
	void SaveRunToDisk();

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Disk")
	bool LoadRunFromDisk(); 

	// 영구 데이터 파일 관리
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Disk")
	void SavePermToDisk();

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Disk")
	bool LoadPermFromDisk();

	//영구 데이터 존재 여부 확인
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Disk")
	bool HasValidPermSave() const;

	//런 데이터 존재 여부 확인
	UFUNCTION(BlueprintPure, Category = "SaveSystem|Disk")
	bool HasValidRunSave() const;

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Sound")
	void SaveSoundSettings(float Master, float BGM, float SFX, float UI);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Flow")
	void MarkIntroAsSeen();

	UFUNCTION(BlueprintPure, Category = "SaveSystem|Flow")
	bool HasSeenIntro() const;
private:
	UPROPERTY()
	FPlayerRunData RunData;

	UPROPERTY()
	FPlayerMetaProgressionData PermData;

	//저장될 파일 이름
	const FString RunSlotName = TEXT("Slot_Run");
	const FString PermSlotName = TEXT("Slot_Perm");
};
