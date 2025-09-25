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
public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> FieldHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> BattleHUDClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UUserWidget> CurrentHUD;

	// 사용할 입력 매핑 컨텍스트
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> FieldIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> PlayerTurnIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> EnemyTurnIMC;

public:
	UFUNCTION(BlueprintCallable)
	void ShowFieldHUD();
	UFUNCTION(BlueprintCallable)
	void ShowBattleHUD();

	void ClearCurrentHUD();

	//입력 방식 교체 함수
	UFUNCTION(BlueprintCallable)
	void SetFieldInputMode();

	UFUNCTION(BlueprintCallable)
	void SetPlayerTurnInputMode();

	UFUNCTION(BlueprintCallable)
	void SetEnemyTurnInputMode();
	
};
