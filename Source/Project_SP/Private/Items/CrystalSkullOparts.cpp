//#include "Items/CrystalSkullOparts.h"
//#include "UObject/ConstructorHelpers.h" // 경로 지정 헬퍼 사용
//
//UCrystalSkullOparts::UCrystalSkullOparts()
//{
//    // 컴포넌트가 기본적으로 틱을 사용하지 않도록 설정
//    PrimaryComponentTick.bCanEverTick = false;
//
//    // 이 컴포넌트의 초기 활성화 상태를 명시적으로 비활성화
//    bWantsInitializeComponent = true;
//    SetComponentTickEnabled(false);
//
//    SetActive(true);
//
//    static ConstructorHelpers::FObjectFinder<UDataTable> DTFinder(TEXT("/Game/DataTable/DT_OpartsStat.DT_OpartsStat"));
//    if (DTFinder.Succeeded())
//    {
//        OpartsStatsDataTable = DTFinder.Object;
//    }
//    else
//    {
//        UE_LOG(LogTemp, Error, TEXT("Failed to load CrystalSkull Data Table!"));
//    }
//
//    OpartsName = FText::FromString(TEXT("크리스탈 해골"));
//
//    // 멀티라인 텍스트 (\r\n으로 줄바꿈)
//    UniqueAbilityTitle = FText::FromString(TEXT("고유 능력: 크리티컬 처치"));
//    UniqueAbilityDesc = FText::FromString(TEXT("• 치명타 1타 당 게이지 20 획득 \r\n• 게이지 100 도달 시, 다음 스킬 피해량 + 100 % "));
//
//    static ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("/Game/Textures/Skull.Skull"));
//
//    if (IconAsset.Succeeded())
//    {
//        OpartsIcon = IconAsset.Object; // 찾은 에셋을 변수에 할당
//    }
//
//    ActiveAbilityDescriptions.Empty();
//
//    // 1단계
//    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("치명타 확률 + 20 %, 공격력 + 20 %")));
//    // 2단계
//    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("치명타 1회당 게이지 획득량 25로 증가")));
//    // 3단계
//    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("메인 스킬 피해량 + 30 %")));
//    // 4단계
//    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("치명타 확률 + 10 %, 치명타 피해 + 30")));
//    // 5단계
//    ActiveAbilityDescriptions.Add(FText::FromString(TEXT("게이지가 최대치가 되었을 때 다음 스킬 피해량 200 % 증가로 변경\r\n메인 스킬 쿨타임 1턴 감소")));
//
//}
