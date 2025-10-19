// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/JadeClockOparts.h"
#include "UObject/ConstructorHelpers.h" // 경로 지정 헬퍼 사용

UJadeClockOparts::UJadeClockOparts()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 이 컴포넌트의 틱 레이트를 비활성화
    bWantsInitializeComponent = true;
    SetComponentTickEnabled(false);

	SetActive(false);

    static ConstructorHelpers::FObjectFinder<UDataTable> DTFinder(TEXT("/Game/DataTable/DT_OpartsStat.DT_OpartsStat"));
    if (DTFinder.Succeeded())
    {
        OpartsStatsDataTable = DTFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load CrystalSkull Data Table!"));
    }
}
