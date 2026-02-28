#include "Map/RewardBox.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"

ARewardBox::ARewardBox()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ARewardBox::ExecuteInteraction(AActor* Interactor)
{
	if (bIsOpened) return;

	// 상호작용한 Actor가 플레이어인지 확인
	APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(Interactor)->GetController());
	if (!PC) return;

	bIsOpened = true;

	//보상 지급 코드를 여기서 구현
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. 유물 보상 위젯 생성
	UUserWidget* RewardUI = CreateWidget<UUserWidget>(PC, RelicRewardWidgetClass);
	if (RewardUI)
	{
		// 2. 위젯에 현재 스테이지 값 전달 (WBP_RelicReward의 CurrentStage 변수 이름과 일치해야 함)
		FProperty* StageProp = RewardUI->GetClass()->FindPropertyByName(FName("CurrentStage"));
		if (StageProp)
		{
			if (FIntProperty* IntProp = CastField<FIntProperty>(StageProp))
			{
				IntProp->SetPropertyValue_InContainer(RewardUI, StageLevel);
			}
		}

		// 3. 화면에 표시 및 입력 모드 설정
		RewardUI->AddToViewport();

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(RewardUI->GetCachedWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AActor* MapActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMapBase::StaticClass());
	if (AMapBase* CurrentMap = Cast<AMapBase>(MapActor))
	{
		CurrentMap->SetMapState(EMapState::Cleared);
	}

	Destroy();
}

FText ARewardBox::GetInteractText() const
{
	return FText::FromString(TEXT("상자 열기"));
}
