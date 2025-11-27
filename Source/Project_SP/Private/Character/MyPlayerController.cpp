// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyPlayerController.h"
#include "UI/RelicSelectWidget.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"

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
