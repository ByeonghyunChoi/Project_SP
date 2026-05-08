// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SPGASGameInstance.h"
#include "GameFramework/GameUserSettings.h"
#include "SubSystem/SPDialogSubsystem.h"

USPGASGameInstance::USPGASGameInstance()
{
	bIsReturnFromGame = false;
}

void USPGASGameInstance::Init()
{
    Super::Init();

    // 1. 대화 서브시스템을 찾아 기본 에셋 테이블을 설정합니다.
    if (USPDialogSubsystem* DialogSubsystem = GetSubsystem<USPDialogSubsystem>())
    {
        if (DefaultDialogAssetTable)
        {
            DialogSubsystem->DefaultAssetTable = DefaultDialogAssetTable;
        }
    }

    if (GEngine != nullptr)
    {
        UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();

        if (UserSettings != nullptr)
        {
            // 전체 퀄리티 레벨을 '중간(Medium)'으로 고정합니다.
            // (0 = Low, 1 = Medium, 2 = High, 3 = Epic, 4 = Cinematic)
            UserSettings->SetOverallScalabilityLevel(1);

            // 변경된 설정을 시스템에 적용하고 저장합니다.
            // bCheckForCommandLineOverrides 파라미터를 true로 주면 명령줄 인수로 들어온 세팅을 무시하지 않습니다.
            UserSettings->ApplySettings(true);
        }
    }
}

void USPGASGameInstance::SetbIsReturnFromGame(bool IsReturn)
{
	bIsReturnFromGame = IsReturn;
}

bool USPGASGameInstance::GetbIsReturnFromGame() const
{
	return bIsReturnFromGame;
}
