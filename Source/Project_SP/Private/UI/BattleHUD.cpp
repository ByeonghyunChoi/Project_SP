// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BattleHUD.h"

ABattleHUD::ABattleHUD()
{
}

void ABattleHUD::BeginPlay()
{
	Super::BeginPlay();
	CreateAndAddWidget(BattleUIWidgetClass);
}

UUserWidget* ABattleHUD::CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (WidgetClass && GetWorld())
	{
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		if (NewWidget)
		{
			NewWidget->AddToViewport(ZOrder);
			ActiveWidgets.Add(NewWidget);
			APlayerController* PlayerController = GetOwningPlayerController();
			if (PlayerController)
			{
				PlayerController->SetShowMouseCursor(true);
				PlayerController->SetInputMode(FInputModeGameAndUI());
			}
			UE_LOG(LogTemp, Log, TEXT("위젯 생성 및 추가 완료: %s"), *WidgetClass->GetName());
			return NewWidget;
		}
	}
	return nullptr;
}

void ABattleHUD::RemoveCurrentWidget(UUserWidget* WidgetToRemove)
{
	if (WidgetToRemove && ActiveWidgets.Contains(WidgetToRemove))
	{
		WidgetToRemove->RemoveFromParent();
		ActiveWidgets.Remove(WidgetToRemove);
		UE_LOG(LogTemp, Log, TEXT("위젯 제거 완료: %s"), *WidgetToRemove->GetName());
		if (ActiveWidgets.Num() == 0) // 모든 위젯 제거 시 마우스 초기화
		{
			APlayerController* PlayerController = GetOwningPlayerController();
			if (PlayerController)
			{
				PlayerController->SetShowMouseCursor(false);
				PlayerController->SetInputMode(FInputModeGameOnly());
			}
		}
	}
}

void ABattleHUD::RemoveAllWidgets()
{
	for (UUserWidget* Widget : ActiveWidgets)
	{
		if (Widget) Widget->RemoveFromParent();
	}
	ActiveWidgets.Empty();
	UE_LOG(LogTemp, Log, TEXT("모든 위젯 제거 완료."));
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

UUserWidget* ABattleHUD::GetWidgetByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	for (UUserWidget* Widget : ActiveWidgets)
	{
		if (Widget && Widget->IsA(WidgetClass)) return Widget;
	}
	return nullptr;
}
