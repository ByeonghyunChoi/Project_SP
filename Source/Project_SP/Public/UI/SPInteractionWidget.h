// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SPInteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateInteractionText(const FText& Text);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Txt_ActionName;
	
};
