// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/GoldBugOparts.h"
#include "UObject/ConstructorHelpers.h" // 경로 지정 헬퍼 사용

UGoldBugOparts::UGoldBugOparts()
{
    // 컴포넌트가 기본적으로 틱을 사용하지 않도록 설정
    PrimaryComponentTick.bCanEverTick = false;

    // 이 컴포넌트의 초기 활성화 상태를 명시적으로 비활성화
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
