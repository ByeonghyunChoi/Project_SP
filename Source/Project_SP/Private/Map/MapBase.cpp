#include "Map/MapBase.h"
#include "Map/MapManagerSubsystem.h"
#include "Map/PortalActor.h"
#include "Components/SceneComponent.h"
#include "Map/RewardBox.h"
#include "Kismet/GameplayStatics.h"
#include "SubSystem/SPSaveGameSubsystem.h"

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

void AMapBase::InitializeMap(EMapType InType, EMapState InitialState)
{
	MapType = InType;
	SetMapState(EMapState::InProgress);

	if (InitialState == EMapState::Reward)
	{
		ClearFieldMonsters();
		SetMapState(EMapState::Reward); // Reward 발동 -> 상자 스폰됨
	}
	else if (InitialState == EMapState::Cleared)
	{
		ClearFieldMonsters();
		// Reward 단계를 건너뛰고 바로 Cleared로 직행! -> 상자 절대 안 나옴! 포탈만 활성화됨!
		SetMapState(EMapState::Cleared);
	}
	else if (MapType == EMapType::Prepare)
	{
		// 진입하자마자 즉시 클리어 상태로 덮어버립니다!
		SetMapState(EMapState::Cleared);
		UE_LOG(LogTemp, Warning, TEXT("[MapBase] 준비 구역 진입: 포탈을 즉시 개방합니다."));
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

	TArray<EMapType> Options = MapManager->GenerateNextFloorOptions();
	SpawnedPortals.Empty();

	bool bIsFinalBoss = (MapManager->GetCurrentStage() >= 2 && MapManager->GetCurrentFloor() >= 5);
	TSubclassOf<APortalActor> ClassToSpawn = (bIsFinalBoss && EndingPortalClass) ? EndingPortalClass : PortalClass;

	if (!ClassToSpawn) return;

	// 🌟 1. 포탈이 딱 1개만 나올 때 (보스, 준비 맵)
	if (Options.Num() == 1)
	{
		// 선생님의 아이디어: 특정 태그(Center)를 가진 스폰 포인트를 찾습니다!
		TArray<FTransform> CenterPoints = GetSpawnTransformsByTag(TEXT("SpawnPoint.Portal.Center"));

		if (CenterPoints.Num() > 0)
		{
			APortalActor* NewPortal = GetWorld()->SpawnActor<APortalActor>(ClassToSpawn, CenterPoints[0]);
			if (NewPortal)
			{
				NewPortal->SetPortalTargetType(Options[0]);
				NewPortal->ActivatePortal(false);
				SpawnedPortals.Add(NewPortal);
			}
			return; // 🌟 1개 스폰에 성공했으니 함수를 깔끔하게 종료!
		}
		else
		{
			// 만약 맵에 Center 태그를 깜빡하고 안 달았다면, 경고 로그를 띄우고 아래의 기본 로직으로 넘어갑니다.
			UE_LOG(LogTemp, Warning, TEXT("[MapBase] Center 포탈 스폰 포인트가 없습니다! 기본 스폰 포인트를 사용합니다."));
		}
	}

	// 🌟 2. 포탈이 여러 개일 때 (또는 Center 태그를 못 찾았을 때)
	TArray<FTransform> SpawnPoints = GetSpawnTransformsByTag(TEXT("SpawnPoint.Portal"));

	for (int32 i = 0; i < FMath::Min(Options.Num(), SpawnPoints.Num()); ++i)
	{
		APortalActor* NewPortal = GetWorld()->SpawnActor<APortalActor>(ClassToSpawn, SpawnPoints[i]);
		if (NewPortal)
		{
			NewPortal->SetPortalTargetType(Options[i]);
			NewPortal->ActivatePortal(false);
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

		ARewardBox* SpawnedChest = GetWorld()->SpawnActor<ARewardBox>(RewardChestClass, RewardPoints[0], SpawnParams);
		if (SpawnedChest)
		{
			SpawnedChest->SetupParticleByMapType(MapType);
		}

		UE_LOG(LogTemp, Log, TEXT("Reward Chest Spawned!"));
	}

	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player)
		{
			SaveSys->CacheRunDataFromPlayer(Player);
			SaveSys->SaveRunToDisk();
			UE_LOG(LogTemp, Log, TEXT("[AutoSave] 보상 방: 전투 승리 상태 저장 완료"));
		}
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

	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		MapManager->SetCurrentRoomState(EMapState::Cleared);
	}

	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player)
		{
			SaveSys->CacheRunDataFromPlayer(Player);
			SaveSys->SaveRunToDisk();

			// 혹시 영구 재화를 먹었을 수도 있으니 Perm도 갱신
			SaveSys->CachePermDataFromPlayer(Player);
			SaveSys->SavePermToDisk();
			UE_LOG(LogTemp, Log, TEXT("[AutoSave] 클리어: 보상 획득 및 포탈 개방 상태 저장 완료"));
		}
	}
}
