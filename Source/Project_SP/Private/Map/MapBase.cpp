#include "Map/MapBase.h"
#include "Map/MapManagerSubsystem.h"
#include "Map/PortalActor.h"
#include "Components/SceneComponent.h"

AMapBase::AMapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CurrentState = EMapState::InProgress;
}

void AMapBase::BeginPlay()
{
	Super::BeginPlay();

	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		MapManager->InitializeCurrentMap(this);
	}
}

TArray<FTransform> AMapBase::GetSpawnTransformsByTag(FName PointTag) const
{
	TArray<FTransform> FoundTransforms;
	TArray<USceneComponent*> Components;
	GetComponents<USceneComponent>(Components);

	for (USceneComponent* Comp : Components)
	{
		if (Comp && Comp->ComponentHasTag(PointTag))
		{
			FoundTransforms.Add(Comp->GetComponentTransform());
		}
	}
	return FoundTransforms;
}

void AMapBase::SetMapState(EMapState NewState)
{
	if (CurrentState == NewState) return;

	EMapState OldState = CurrentState;
	CurrentState = NewState;

	switch (CurrentState)
	{
	case EMapState::InProgress:
		HandleStateInProgress();
		break;
	case EMapState::Reward:
		HandleStateReward();
		break;
	case EMapState::Cleared:
		HandleStateCleared();
		break;
	}

	OnMapStateChanged(OldState, NewState);
}

void AMapBase::InitializeMap(EMapType InType)
{
	MapType = InType;
	SetMapState(EMapState::InProgress);
}

void AMapBase::HandleStateInProgress()
{
	//맵의 시작 로직 수행, 기믹 배치 혹은 몬스터 스폰, 포탈 스폰(비활성화)
}

void AMapBase::HandleStateReward()
{
	//전투 승리 후, 또는 맵 로직 완수 후 호출 될 로직
}

void AMapBase::HandleStateCleared()
{
	UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
	if (!MapManager || !PortalClass) return;

	// 다음 층 선택지 생성 및 포탈 스폰
	TArray<EMapType> Options = MapManager->GenerateNextFloorOptions();
	TArray<FTransform> SpawnPoints = GetSpawnTransformsByTag(TEXT("Portal"));

	for (int32 i = 0; i < FMath::Min(Options.Num(), SpawnPoints.Num()); ++i)
	{
		APortalActor* NewPortal = GetWorld()->SpawnActor<APortalActor>(PortalClass, SpawnPoints[i]);
		if (NewPortal) NewPortal->SetPortalTargetType(Options[i]);
	}
}
