// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> FieldHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> BattleHUDClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UUserWidget> CurrentHUD;

public:
	UFUNCTION(BlueprintCallable)
	void ShowFieldHUD();
	UFUNCTION(BlueprintCallable)
	void ShowBattleHUD();

	void ClearCurrentHUD();
	
};
