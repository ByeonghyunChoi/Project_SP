// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API ABattleHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ABattleHUD();

	//전투 시작시 기본 UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> BattleUIWidgetClass;

	//위젯들을 관리할 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<UUserWidget*> ActiveWidgets;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RemoveCurrentWidget(UUserWidget* WidgetToRemove);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RemoveAllWidgets();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	UUserWidget* GetWidgetByClass(TSubclassOf<UUserWidget> WidgetClass) const;
};
