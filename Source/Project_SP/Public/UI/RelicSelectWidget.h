// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/RelicData.h"
#include "RelicSelectWidget.generated.h"

/**
 *  유물 선택 화면의 부모 클래스
 */
UCLASS()
class PROJECT_SP_API URelicSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 컨트롤러가 이 함수를 호출해 데이터를 넘겨줌.
	// 실제 구현(화면에 띄우기)은 블루프린트(WBP_RelicSelect)에서 함.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Relic UI")
	void SetupRelicOptions(const TArray<FRelicData>& RelicOptions);
};
