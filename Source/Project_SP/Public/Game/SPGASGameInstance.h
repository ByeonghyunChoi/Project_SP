// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SPGASGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGASGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USPGASGameInstance();
	virtual void Init() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	TObjectPtr<class UDataTable> DefaultDialogAssetTable;
private:
	bool bIsReturnFromGame;

public:
	UFUNCTION(BlueprintCallable)
	void SetbIsReturnFromGame(bool IsReturn);
	
	UFUNCTION(BlueprintCallable)
	bool GetbIsReturnFromGame() const;
};
