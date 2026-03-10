#include "Map/MapBase.h"
#include "Map/MapManagerSubsystem.h"
#include "Map/PortalActor.h"
#include "Components/SceneComponent.h"
#include "Map/RewardBox.h"
#include "Kismet/GameplayStatics.h"

AMapBase::AMapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CurrentState = EMapState::None;
}

void AMapBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMapBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 내가 관리하던 포탈들 싹 다 제거
	for (APortalActor* Portal : SpawnedPortals)
	{
		// 포탈이 유효하다면(아직 월드에 있다면) 파괴
		if (IsValid(Portal))
		{
			Portal->Destroy();
		}
	}

	// 배열 비우기
	SpawnedPortals.Empty();

	UE_LOG(LogTemp, Log, TEXT("MapBase Destroyed: All Portals Cleaned up."));
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

void AMapBase::InitializeMap(EMapType InType, bool bIsCleared)
{
	MapType = InType;
	SetMapState(EMapState::InProgress);

	if (bIsCleared)
	{
		ClearFieldMonsters(); 
		SetMapState(EMapState::Reward); 
	}
}

void AMapBase::ClearFieldMonsters()
{
	TArray<AActor*> FieldMonsters;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FieldMonsters);

	for (AActor* Monster : FieldMonsters)
	{
		if (IsValid(Monster))
		{
			Monster->Destroy();
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("방이 클리어되어 필드 몬스터를 모두 청소했습니다."));
}

void AMapBase::HandleStateInProgress()
{
	UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
	if (!MapManager || !PortalClass) return;

	// 다음 층 선택지 미리 계산
	TArray<EMapType> Options = MapManager->GenerateNextFloorOptions();
	TArray<FTransform> SpawnPoints = GetSpawnTransformsByTag(TEXT("SpawnPoint.Portal"));

	SpawnedPortals.Empty();

	// 포탈 스폰 (비활성화)
	for (int32 i = 0; i < FMath::Min(Options.Num(), SpawnPoints.Num()); ++i)
	{
		APortalActor* NewPortal = GetWorld()->SpawnActor<APortalActor>(PortalClass, SpawnPoints[i]);
		if (NewPortal)
		{
			NewPortal->SetPortalTargetType(Options[i]);
			NewPortal->ActivatePortal(false); //비활성화 상태로 시작
			SpawnedPortals.Add(NewPortal);
		}
	}
}

void AMapBase::HandleStateReward()
{
	if (!RewardChestClass)
	{
		// 상자가 없으면 바로 클리어 처리
		SetMapState(EMapState::Cleared);
		return;
	}

	TArray<FTransform> RewardPoints = GetSpawnTransformsByTag(TEXT("SpawnPoint.Reward"));
	UE_LOG(LogTemp, Warning, TEXT("Reward Points Found: %d"), RewardPoints.Num());
	if (RewardPoints.Num() > 0)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<ARewardBox>(RewardChestClass, RewardPoints[0], SpawnParams); // 파라미터 전달

		UE_LOG(LogTemp, Log, TEXT("Reward Chest Spawned!"));
	}
}

void AMapBase::HandleStateCleared()
{
	for (APortalActor* Portal : SpawnedPortals)
	{
		if (Portal)
		{
			Portal->ActivatePortal(true);
		}
	}
}
