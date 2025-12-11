// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyPlayerController.h"
#include "UI/RelicSelectWidget.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Map/MapManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    SetFieldInputMode();
}

void AMyPlayerController::ShowFieldHUD()
{
    ClearCurrentHUD(); // 기존 HUD 제거
    if (FieldHUDClass)
    {
        CurrentHUD = CreateWidget<UUserWidget>(this, FieldHUDClass);
        if (CurrentHUD)
        {
            CurrentHUD->AddToViewport();
        }
    }
}

void AMyPlayerController::ShowBattleHUD()
{
    ClearCurrentHUD(); // 기존 HUD 제거
    if (BattleHUDClass)
    {
        CurrentHUD = CreateWidget<UUserWidget>(this, BattleHUDClass);
        if (CurrentHUD)
        {
            CurrentHUD->AddToViewport();
        }
    }
}

void AMyPlayerController::ClearCurrentHUD()
{
    if (CurrentHUD)
    {
        CurrentHUD->RemoveFromParent();
        CurrentHUD = nullptr;
    }
}

void AMyPlayerController::SetFieldInputMode()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(FieldIMC, 0);
    }
}

void AMyPlayerController::SetPlayerTurnInputMode()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(PlayerTurnIMC, 0);
    }
}

void AMyPlayerController::SetEnemyTurnInputMode()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(EnemyTurnIMC, 0);
    }
}

void AMyPlayerController::ShowRelicSelectionUI(const TArray<FRelicData>& Choices)
{
    // 1. 위젯 클래스가 할당되어 있는지 확인
    if (!RelicSelectWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("RelicSelectWidgetClass is not set in MyPlayerController!"));
        return;
    }

    // 2. 위젯 생성 (URelicSelectWidget 타입으로 캐스팅하여 생성)
    // CreateWidget<타입>(OwningObject, Class)
    URelicSelectWidget* RelicWidget = CreateWidget<URelicSelectWidget>(this, RelicSelectWidgetClass);

    if (RelicWidget)
    {
        // 3. 뷰포트에 추가 (ZOrder를 100 정도로 주어 다른 UI보다 위에 뜨게 설정)
        RelicWidget->AddToViewport(100);

        // 4. [핵심] C++ 데이터를 블루프린트 위젯으로 전달
        // 이 함수가 실행되면 WBP의 'Event Setup Relic Options' 노드가 호출됩니다.
        RelicWidget->SetupRelicOptions(Choices);

        // 5. 입력 모드 변경 (UI 전용)
        // 유물을 고르는 동안에는 캐릭터가 움직이면 안 되므로 UI 입력만 받습니다.
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(RelicWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        SetInputMode(InputMode);
        SetShowMouseCursor(true); // 마우스 커서 보이기

        // (선택 사항) 게임 일시 정지 - 선택하는 동안 몬스터가 때리면 안 되니까
        // SetPause(true); 

        UE_LOG(LogTemp, Log, TEXT("유물 선택 UI가 표시되었습니다."));
    }
}

void AMyPlayerController::InitStageUI()
{
    // 1. 이미 있다면 제거 (재시작 시 중복 방지)
    if (StageWidgetInstance)
    {
        StageWidgetInstance->RemoveFromParent();
        StageWidgetInstance = nullptr;
    }

    // 2. 위젯 생성
    if (!StageWidgetClass) return;
    StageWidgetInstance = CreateWidget<UW_StageProgress>(this, StageWidgetClass);

    if (StageWidgetInstance)
    {
        StageWidgetInstance->AddToViewport();

        // 3. 데이터 가져오기 (이 시점엔 StartNewRun이 끝난 직후라 데이터가 깨끗함)
        UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
        if (MapManager)
        {
            TArray<EMapType> MapLayout;
            int32 CurrentIndex = 0;

            // 데이터 조회
            MapManager->GetCurrentStageLayout(MapLayout, CurrentIndex);

            // UI 초기화 (1-1 스테이지로 세팅)
            StageWidgetInstance->InitializeMap(1, CurrentIndex + 1, MapLayout, CurrentIndex);
        }
    }
}

void AMyPlayerController::UpdateStageUI()
{
    // 플레이어가 이동한 뒤에 호출할 함수
    if (StageWidgetInstance)
    {
        UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
        if (MapManager)
        {
            TArray<EMapType> DummyLayout;
            int32 NewIndex = 0;

            // 현재 위치 인덱스만 다시 가져옴
            MapManager->GetCurrentStageLayout(DummyLayout, NewIndex);

            // 화살표 갱신
            StageWidgetInstance->UpdatePlayerPosition(NewIndex);
        }
    }
}
