
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SPDialogData.h"
#include "SPDialogSubsystem.generated.h"

// 서브시스템이 대사 데이터가 업데이트될 때마다 UI에 알릴 수 있도록 델리게이트 선언 - UI 연출이 필요해서
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogRowUpdated, const FDialogLineData&, DialogRowData, const FDialogAssetData&, DialogAssetData);
// 대화가 끝났을때를 알려주기 위해
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogFinished);

//대화 시작 시 NPC 이미지를 미리 띄우기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogStandingSetup, const FDialogAssetData&, NPCAssetData);


UCLASS()
class PROJECT_SP_API USPDialogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 대화 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void StartDialog(UDataTable* DialogTable, UDataTable* AssetTable, APlayerController* PC, TSubclassOf<class UUserWidget> DialogWidgetClass);
	// 대화 바로 띄우기 함수
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void PlayNextDialog();
	// 3. 대화 종료 함수
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void EndDialog();

public:
	// 블루프린트(UI)에서 바인딩할 수 있도록
	UPROPERTY(BlueprintAssignable, Category = "Dialog")
	FOnDialogRowUpdated OnDialogRowUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Dialog")
	FOnDialogFinished OnDialogFinished;

	// 에디터에서 미리 지정해둘 기본 에셋 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	TObjectPtr<UDataTable> DefaultAssetTable;

	// 블루프린트에서 바인딩할 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Dialog")
	FOnDialogStandingSetup OnDialogStandingSetup;

private:
	//현재 읽고 있는 테이블의 '모든 행'을 순서대로 담아둘 배열
	TArray<FDialogLineData> CurrentDialogRows; // 검색으로 띄우는 대신 배열에 담아놓고 +1씩으로 편하기 읽어나가기 위함

	// 현재 사용 중인 에셋 
	UPROPERTY()
	TObjectPtr<UDataTable> CurrentAssetTable;

	// 현재 배열의 몇 번째 줄을 읽고 있는지 기억하는 책갈피 역할
	int32 CurrentRowIndex = 0; // 현재 텍스트를 바로 띄우기 위해서

	// 생성된 대화창 UI(위젯)를 기억해두는 변수 // 대화가 끝나면 이 위젯을 제거
	UPROPERTY()
	TObjectPtr<class UUserWidget> ActiveDialogWidget;
};
