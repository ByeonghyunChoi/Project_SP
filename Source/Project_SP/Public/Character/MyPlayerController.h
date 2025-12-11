// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/RelicData.h"
#include "UI/W_StageProgress.h"
#include "MyPlayerController.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class PROJECT_SP_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> FieldHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> BattleHUDClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UUserWidget> CurrentHUD;

	// 사용할 입력 매핑 컨텍스트
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> FieldIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> PlayerTurnIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> EnemyTurnIMC;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UW_StageProgress> StageWidgetClass;

	UPROPERTY()
	TObjectPtr<UW_StageProgress> StageWidgetInstance;

public:
	UFUNCTION(BlueprintCallable)
	void ShowFieldHUD();
	UFUNCTION(BlueprintCallable)
	void ShowBattleHUD();

	void ClearCurrentHUD();

	//입력 방식 교체 함수
	UFUNCTION(BlueprintCallable)
	void SetFieldInputMode();

	UFUNCTION(BlueprintCallable)
	void SetPlayerTurnInputMode();

	UFUNCTION(BlueprintCallable)
	void SetEnemyTurnInputMode();
	
	// 유물 선택 UI 관련
	// 에디터에서 WBP_RelicSelect를 할당할 변수
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> RelicSelectWidgetClass;

	// 맵(보상상자)에서 호출할 함수: 유물 데이터 3개를 받아 UI를 켭니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowRelicSelectionUI(const TArray<FRelicData>& Choices);

	//맵 스테이지 UI 호출 함수
	void InitStageUI();
	//맵 스테이지 UI 갱신 함수
	void UpdateStageUI();

	//----------시스템 메뉴 관련 변수/함수들----------
public:
	// 에디터에서 WBP_SystemMenu를 넣을 변수
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> SystemMenuClass;

	// 생성된 위젯을 담아둘 변수
	UPROPERTY()
	class UUserWidget* SystemMenuWidget;

	// ESC 키를 누르면 실행될 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleSystemMenu();
};
