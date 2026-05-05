

#include "SubSystem/SPDialogSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void USPDialogSubsystem::StartDialog(UDataTable* DialogTable, APlayerController* PC, TSubclassOf<UUserWidget> DialogWidgetClass)
{
	// 방어 코드: 꼭 필요한 재료(테이블, 컨트롤러, UI클래스)가 없으면 종료 - 괜히 재료 없이 대화 시작 X - 점수를 잘 받아야 하는것이지 아무래도
	if (!DialogTable || !PC || !DialogWidgetClass) return;

	// 1. 기존 데이터 청소.
	CurrentDialogRows.Empty();
	CurrentRowIndex = 0;

	// 2. 데이터 테이블의 모든 내용을 배열(CurrentDialogRows) 받아오기
	TArray<FDialogRow*> AllRows;
	DialogTable->GetAllRows<FDialogRow>(TEXT("DialogContext"), AllRows);

	for (FDialogRow* Row : AllRows)
	{
		if (Row)
		{
			CurrentDialogRows.Add(*Row);
		}
	}

	// 3. UI(위젯) 생성 및 띄우기
	if (!ActiveDialogWidget)
	{
		ActiveDialogWidget = CreateWidget<UUserWidget>(PC, DialogWidgetClass);
	}

	if (ActiveDialogWidget && !ActiveDialogWidget->IsInViewport())
	{
		ActiveDialogWidget->AddToViewport(100); // ZOrder 100을 줘서 다른 UI(체력바 등)보다 항상 맨 위에 뜨게

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(ActiveDialogWidget->GetCachedWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	// 4. 모든 준비가 끝났으니 첫 번째 대사를 출력하라고 지시합니다.
	PlayNextDialog();
}

void USPDialogSubsystem::PlayNextDialog()
{
	// 다음 대사가 남아있는지 확인
	if (CurrentDialogRows.IsValidIndex(CurrentRowIndex))
	{
		// 1. UI에게 "이게 전체 텍스트야!" 라고 방송
		OnDialogRowUpdated.Broadcast(CurrentDialogRows[CurrentRowIndex]);

		// 2. 책갈피 다음 장으로 넘기기
		CurrentRowIndex++;
	}
	else
	{
		// 대사가 다 끝났다면 대화 종료!
		EndDialog();
	}
}

void USPDialogSubsystem::EndDialog()
{
	if (ActiveDialogWidget)
	{
		ActiveDialogWidget->RemoveFromParent();
		ActiveDialogWidget = nullptr;
	}

	// 조작 권한 원상복구
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		FInputModeGameOnly GameMode;
		PC->SetInputMode(GameMode);
		PC->SetShowMouseCursor(false);
	}

	// 퀘스트 시스템 등에게 대화 종료 알림
	OnDialogFinished.Broadcast();
}