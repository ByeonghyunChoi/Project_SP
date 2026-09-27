// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SPInteractionWidget.h"
#include "Components/TextBlock.h"

void USPInteractionWidget::UpdateInteractionText(const FText& Text)
{
	if (Txt_ActionName)
	{
		Txt_ActionName->SetText(FText::Format(NSLOCTEXT("SPInteractionWidget", "InteractionPrompt", "[F] {0}"), Text));
	}
}

void USPInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Txt_ActionName)
	{
		Txt_ActionName->SetText(FText::GetEmpty());
	}
}
