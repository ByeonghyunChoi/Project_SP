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

    OpartsName = FText::FromString(TEXT("골드버그"));

    // 멀티라인 텍스트 (\r\n으로 줄바꿈)
    UniqueAbilityTitle = FText::FromString(TEXT("고유 능력: 상태이상 강화"));
    UniqueAbilityDesc = FText::FromString(TEXT("• 상태이상 융합 가능 \r\n• 상태이상 융합 4회마다 추가 턴 진행  \r\n (해당 추가 턴에서 직접 가하는 데미지 60 % 하락) "));

    static ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("/Game/Textures/GoldBug.GoldBug"));

    if (IconAsset.Succeeded())
    {
        OpartsIcon = IconAsset.Object; // 찾은 에셋을 변수에 할당
    }

    // 1단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("속도 20 % 증가, 공격력 30 % 증가")));
    // 2단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("스킬 효과 적용 확률 40 % 증가")));
    // 3단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("효과 효율 50 % 증가")));
    // 4단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("공격력 20 % 증가, 속도 15 % 증가")));
    // 5단계
    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("상태 이상 융합 3회마다 추가 턴 진행 \r\n효과 효율 50 % 증가")));

}
