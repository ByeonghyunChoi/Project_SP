

#include "SubSystem/SPDialogSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void USPDialogSubsystem::StartDialog(UDataTable* DialogTable, UDataTable* AssetTable, APlayerController* PC, TSubclassOf<UUserWidget> DialogWidgetClass)
{
	// 1. 사용할 에셋 테이블 결정 (입력값이 있으면 그것을, 없으면 기본값을 사용)
	CurrentAssetTable = AssetTable ? AssetTable : DefaultAssetTable.Get();

	// 필수 재료 확인
	if (!DialogTable || !CurrentAssetTable || !PC || !DialogWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartDialog: Missing required data (Table, PC, or WidgetClass)"));
		return;
	}

	// 2. 기존 데이터 초기화
	CurrentDialogRows.Empty();
	CurrentRowIndex = 0;

	// 3. 데이터 테이블의 모든 대사 데이터를 배열로 복사
	TArray<FDialogLineData*> AllRows;
	DialogTable->GetAllRows<FDialogLineData>(TEXT("DialogContext"), AllRows);

	for (FDialogLineData* Row : AllRows)
	{
		if (Row)
		{
			CurrentDialogRows.Add(*Row);
		}	
	}

	// 4. UI 위젯 생성 및 출력
	if (!ActiveDialogWidget)
	{
		ActiveDialogWidget = CreateWidget<UUserWidget>(PC, DialogWidgetClass);
	}

	if (ActiveDialogWidget && !ActiveDialogWidget->IsInViewport())
	{
		ActiveDialogWidget->AddToViewport(100);

		// 입력 모드를 UI 전용으로 변경
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(ActiveDialogWidget->GetCachedWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		// 유령 입력 방지
		PC->FlushPressedKeys();

		// 캐릭터의 이동 명령까지 확실히 정지
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			PlayerPawn->ConsumeMovementInputVector();
		}
	}

	//NPC 이미지 선행 로드
	if (CurrentAssetTable)
	{
		for (const FDialogLineData& Row : CurrentDialogRows)
		{
			// 주인공이 아닌 캐릭터를 발견했다면?
			if (Row.SpeakerID != FName("SI_kardin"))
			{
				FString ContextString = TEXT("FindNPCAsset");
				FDialogAssetData* NPCAsset = CurrentAssetTable->FindRow<FDialogAssetData>(Row.SpeakerID, ContextString);

				// 해당 NPC의 에셋을 찾아서 UI에 "미리 띄워!" 하고 방송합니다.
				if (NPCAsset)
				{
					OnDialogStandingSetup.Broadcast(*NPCAsset);
				}
				else
				{
					//  찾지 못했다면 빈 에셋 데이터를 억지로 만들어서 방송합니다!
					FDialogAssetData EmptyAsset;
					OnDialogStandingSetup.Broadcast(EmptyAsset);
					UE_LOG(LogTemp, Warning, TEXT("[Dialog] %s 의 일러스트를 찾지 못해 빈 이미지를 송출합니다."), *Row.SpeakerID.ToString());
				}
				break; // 찾았으니 반복문 즉시 종료!
			}
		}
	}

	// 5. 첫 번째 대사 즉시 실행
	PlayNextDialog();

	//UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void USPDialogSubsystem::PlayNextDialog()
{
	if (CurrentDialogRows.IsValidIndex(CurrentRowIndex))
	{
		// 1. 현재 출력할 대사 데이터 가져오기
		const FDialogLineData& LineData = CurrentDialogRows[CurrentRowIndex];

		// 2. 에셋 테이블에서 SpeakerID(FName)를 키값으로 에셋 정보를 찾음
		FDialogAssetData AssetData;
		if (CurrentAssetTable)
		{
			FString ContextString = TEXT("FindDialogAsset");
			FDialogAssetData* FoundRow = CurrentAssetTable->FindRow<FDialogAssetData>(LineData.SpeakerID, ContextString);

			if (FoundRow)
			{
				AssetData = *FoundRow;
			}
		}

		// 3. UI로 데이터 방송 (텍스트 + 에셋)
		OnDialogRowUpdated.Broadcast(LineData, AssetData);

		// 4. 다음 인덱스로 이동
		CurrentRowIndex++;
	}
	else
	{
		// 더 이상 대사가 없으면 종료
		EndDialog();
	}
}

void USPDialogSubsystem::EndDialog()
{
	//UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (ActiveDialogWidget)
	{
		ActiveDialogWidget->RemoveFromParent();
		ActiveDialogWidget = nullptr;
	}

	// 조작 권한 원상복구
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		FInputModeGameAndUI GameMode;
		PC->SetInputMode(GameMode);
		PC->SetShowMouseCursor(true);
	}

	// 퀘스트 시스템 등에게 대화 종료 알림
	OnDialogFinished.Broadcast();

	// 보스 전투 전 대화 인지 판단해서 전투 진입 코드 로직 넣기
}