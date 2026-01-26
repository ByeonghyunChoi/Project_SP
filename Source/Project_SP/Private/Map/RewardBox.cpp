#include "Map/RewardBox.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"

ARewardBox::ARewardBox()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ARewardBox::ExecuteInteraction(AActor* Interactor)
{
	if (bIsOpened) return;

	bIsOpened = true;

	//보상 지급 코드를 여기서 구현

	AActor* MapActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMapBase::StaticClass());
	if (AMapBase* CurrentMap = Cast<AMapBase>(MapActor))
	{
		CurrentMap->SetMapState(EMapState::Cleared);
	}

	Destroy();
}

FText ARewardBox::GetInteractText() const
{
	return FText();
}
