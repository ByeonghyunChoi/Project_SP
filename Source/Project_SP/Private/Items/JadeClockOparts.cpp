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

    OpartsName = FText::FromString(TEXT("옥시계"));

    // 멀티라인 텍스트 (\r\n으로 줄바꿈)
    UniqueAbilityTitle = FText::FromString(TEXT("고유 능력: 패링강화"));
    UniqueAbilityDesc = FText::FromString(TEXT("• 패링 스킬의 피해량이 50 % 증가 \r\n• 메인 스킬의 피해량이 30 % 하락 "));

    static ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("/Game/Textures/Jade.Jade"));

    if (IconAsset.Succeeded())
    {
        OpartsIcon = IconAsset.Object; // 찾은 에셋을 변수에 할당
    }

    // 1단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("체력 20 % 증가, 공격력 30 % 증가")));
    // 2단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("약점에 맞는 무기로 패링 시도시 \r\n실패해도 패링 스킬 시전하며 BP 1 회복")));
    // 3단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("패링 스킬의 피해량 30 % 증가")));
    // 4단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("평타로 회복되는 BP 1증가")));
    // 5단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("패링 성공시 최대 HP의 15 % 회복 \r\n패링 스킬 피해량 20 % 증가 \r\n 패링 스킬 쿨타임 제거")));


}
