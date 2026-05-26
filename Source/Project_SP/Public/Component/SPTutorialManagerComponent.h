// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tag/SPGameplayTags.h"
#include "SPTutorialManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API USPTutorialManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USPTutorialManagerComponent();

	// 1. 튜토리얼 제어용 함수
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorialScenario();

	// 전투 흐름에 따라 호출되는 트리거들
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnPlayerTurnStarted();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnParryTimingTriggered();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AdvanceStep();

	// 입력 검문소 (컨트롤러에서 매번 호출하여 가로챌지 결정)
	bool CanProcessInput(FGameplayTag InputTag) const;

protected:
	// 현재 각본 단계
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	int32 CurrentStep = 1;

	// 각본이 진행 중인가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	bool bIsTutorialActive = false;

	// 팝업 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|UI")
	TSubclassOf<class UUserWidget> TutorialPopupClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> ActivePopupWidget;

private:
	// 각 스텝별 연출 및 UI 세팅 수행
	void ProcessStep();
};
