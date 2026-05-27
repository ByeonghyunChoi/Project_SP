// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tag/SPGameplayTags.h"
#include "SPTutorialManagerComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepChanged, int32, NewStep);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API USPTutorialManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USPTutorialManagerComponent();

	// 1. 튜토리얼 제어
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorialScenario();

	// 튜토리얼 종료
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void EndTutorial();

	// 다음 단계로 각본 넘기기
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AdvanceStep();

	// 2. 외부 트리거 (게임 모드나 턴 매니저가 특정 상황에 호출해 줌)
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnPlayerTurnStarted();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnParryTimingTriggered();

	// 3. 입력 검문소 (PlayerController가 키를 누를 때마다 물어볼 함수)
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool CanProcessInput(FGameplayTag InputTag) const;

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetCurrentStep() const { return CurrentStep; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTutorialActive() const { return bIsTutorialActive; }

public:
	// 위젯에서 이 이벤트에 바인딩하여 텍스트와 구멍 위치를 업데이트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Tutorial|Event")
	FOnTutorialStepChanged OnTutorialStepChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	int32 CurrentStep = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	bool bIsTutorialActive = false;

	// 화면에 띄울 팝업 위젯 클래스 (블루프린트에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|UI")
	TSubclassOf<class UUserWidget> TutorialPopupClass;

	// 실제로 생성된 팝업 위젯 메모리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UUserWidget> ActivePopupWidget;

private:
	// 내부 상태 갱신 및 UI 업데이트 지시
	void ProcessCurrentStep();

};
