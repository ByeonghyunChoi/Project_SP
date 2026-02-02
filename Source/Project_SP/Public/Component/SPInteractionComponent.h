// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SPInteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API USPInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USPInteractionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* GetCurrentInteractable() const { return CurrentInteractable.Get(); }

protected:
	void PerformTrace();

protected:
	// 감지 설정
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionRadius = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bShowDebug = false;

	// UI 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TSubclassOf<class UUserWidget> InteractionWidgetClass;
	
private:
	// 현재 보고 있는 대상
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentInteractable;

	// 생성된 UI 인스턴스
	UPROPERTY()
	TObjectPtr<class USPInteractionWidget> InteractionWidgetInstance;
};
