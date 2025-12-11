// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/CombatTypes.h"
#include "W_StageNode.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UW_StageNode : public UUserWidget
{
	GENERATED_BODY()

public:
    // 초기화 함수
    UFUNCTION(BlueprintCallable)
    void SetupNode(EMapType NodeType, bool bIsCurrent);

    // 현재 위치인지 갱신
    void SetIsCurrent(bool bIsCurrent);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> NodeImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ArrowImage;

    // 에디터에서 지정할 색상들
    UPROPERTY(EditDefaultsOnly, Category = "Appearance")
    FLinearColor NormalColor = FLinearColor::Blue;

    UPROPERTY(EditDefaultsOnly, Category = "Appearance")
    FLinearColor EpicColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Appearance")
    FLinearColor PrepareColor = FLinearColor::Yellow;

    UPROPERTY(EditDefaultsOnly, Category = "Appearance")
    FLinearColor BossColor = FLinearColor::Red;
	
};
