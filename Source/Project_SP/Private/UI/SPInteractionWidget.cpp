// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SPInteractionWidget.h"
#include "Components/TextBlock.h"

void USPInteractionWidget::UpdateInteractionText(const FText& Text)
{
	if (Txt_ActionName)
	{
		Txt_ActionName->SetText(Text);
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
