// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/MapManagerSubsystem.h"
#include "Map/MapGraphGenerator.h"
#include "Map/MapNode.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerCharacter.h"
#include "Engine/TargetPoint.h"
#include "SubSystem/TimeForceSubsystem.h"

void UMapManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//맵 생성기 인스턴스 생성
	MapGenerator = NewObject<UMapGraphGenerator>(this);

	const FString DataTabletPath = TEXT("/Script/Engine.DataTable'/Game/DataTable/DT_MapData.DT_MapData'");
	MapTypeData = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTabletPath));
	if (MapTypeData)
	{
		UE_LOG(LogTemp, Log, TEXT("맵 데이터 로딩 성공"));
	}

	HubSpawnPointTag = "HubStart";
	MaxStages = 3;
	DungeonSpawnPointTag = "LogStart";
}

void UMapManagerSubsystem::StartNewRun()
{
	if (UTimeForceSubsystem* TimeManager = GetGameInstance()->GetSubsystem<UTimeForceSubsystem>())
	{
		TimeManager->ResetTimeForce();
	}

	CurrentStage = 1;
	ClearedNodeIDs.Empty();
	CurrentNode = nullptr;
	CurrentMapActorInstance = nullptr;

	GenerateNewStageGraph();

	TravelToNode(GraphRoot);
}

void UMapManagerSubsystem::ReturnToHub(bool bPlayerWon)
{
	if (bPlayerWon)
	{
		if (UTimeForceSubsystem* TimeManager = GetGameInstance()->GetSubsystem<UTimeForceSubsystem>())
		{
			TimeManager->ResetTimeForce();
		}
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 현재 맵 파괴
	if (CurrentMapActorInstance)
	{
		CurrentMapActorInstance->Destroy();
		CurrentMapActorInstance = nullptr;
	}

	// 맵 데이터 초기화
	GraphRoot = nullptr;
	CurrentNode = nullptr;
	ClearedNodeIDs.Empty();
	CurrentStage = 1;

	// 플레이어 폰 찾기
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerPawn);

	// 허브 스폰 지점(ATargetPoint) 찾기
	AActor* HubSpawnPoint = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(World, ATargetPoint::StaticClass(), HubSpawnPointTag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		HubSpawnPoint = FoundActors[0]; 
	}

	if (Player && HubSpawnPoint)
	{
		// 3. 텔레포트
		FVector Location = HubSpawnPoint->GetActorLocation();
		FRotator Rotation = HubSpawnPoint->GetActorRotation();
		PlayerPawn->SetActorLocationAndRotation(Location, Rotation);
	}
}

void UMapManagerSubsystem::TravelToNode(UMapNode* TargetNode)
{
	if (UTimeForceSubsystem* TimeManager = GetGameInstance()->GetSubsystem<UTimeForceSubsystem>())
	{
		if (!TimeManager->DecreaseTimeForce(1))
		{
			// 시간의 힘 소모 실패 (게임 오버됨)
			// TimeForceSubsystem이 ReturnToHub를 호출했으므로, 맵 이동을 즉시 중단.
			return;
		}
	}

	UWorld* World = GetWorld();
	
	if (!TargetNode)
	{
		UE_LOG(LogTemp, Log, TEXT("포탈이 없습니다."));
		return;
	}

	if (!MapTypeData)
	{
		UE_LOG(LogTemp, Log, TEXT("맵 타입이 없습니다."));
		return;
	}

	if (!World)
	{
		UE_LOG(LogTemp, Log, TEXT("월드를 찾을 수 없습니다."));
		return;
	}

	//현재 맵 파괴
	if (CurrentMapActorInstance)
	{
		CurrentMapActorInstance->Destroy();
		CurrentMapActorInstance = nullptr;
	}

	//상태 갱신
	if (CurrentNode)
	{
		ClearedNodeIDs.Add(CurrentNode->NodeID); // 이전 노드를 클리어 처리
	}
	CurrentNode = TargetNode; // 현재 위치를 타겟 노드로 변경

	//맵 타입에 맞는 맵 액터 찾기
	const FName RowName = UEnum::GetValueAsName(CurrentNode->MapType);
	FMapDataRow* Row = MapTypeData->FindRow<FMapDataRow>(RowName, TEXT(""));
	if (!Row || !Row->MapClass)
	{
		UE_LOG(LogTemp, Error, TEXT("MapManager: MapTypeData에 '%s' 타입이 정의되지 않았습니다!"), *RowName.ToString());
		// 안전장치로 기본 맵 스폰
		return;
	}
	TSubclassOf<AMapBase> ClassToSpawn = Row->MapClass;

	//스폰 위치 설정
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	AActor* DungeonSpawnPoint = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(World, ATargetPoint::StaticClass(), DungeonSpawnPointTag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		DungeonSpawnPoint = FoundActors[0];
		SpawnLocation = DungeonSpawnPoint->GetActorLocation();
		SpawnRotation = DungeonSpawnPoint->GetActorRotation();
	}

	//새 맵 액터 스폰
	CurrentMapActorInstance = World->SpawnActor<AMapBase>(ClassToSpawn, SpawnLocation, SpawnRotation);
	if (!CurrentMapActorInstance)
	{
		UE_LOG(LogTemp, Fatal, TEXT("MapManager: 맵 스폰에 치명적인 실패가 발생했습니다!"));
		return;
	}

	//데이터 전달
	CurrentMapActorInstance->SetMapType(CurrentNode->MapType);
	CurrentMapActorInstance->InitializeNextNodes(CurrentNode->ChildNodes);

	//플레이어 이동
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Pawn);
	if (PlayerCharacter)
	{
		FVector StartLocation = CurrentMapActorInstance->GetPlayerStartLocation();
		FRotator StartRotation = CurrentMapActorInstance->GetPlayerStartRotation();
		PlayerCharacter->SetActorLocationAndRotation(StartLocation, StartRotation);
	}

	//새 맵의 로직 시작
	CurrentMapActorInstance->BeginMapLogic();
}

void UMapManagerSubsystem::GoToNextStage()
{
	CurrentStage++;

	//마지막 보스를 클리어 했다면 게임 시작 맵으로 이동
	if (CurrentStage > MaxStages)
	{
		ReturnToHub(true); 
		return;
	}

	// 다음 스테이지 맵 그래프 생성
	GenerateNewStageGraph();

	//루트 맵으로 이동(2-1, 3-1)
	TravelToNode(GraphRoot);
}

void UMapManagerSubsystem::NotifyCombatFinished(bool bPlayerWon)
{
	if (CurrentMapActorInstance)
	{
		// 현재 스폰된 맵 액터(예: BP_NormalMap)의 OnCombatFinished 이벤트를 호출
		CurrentMapActorInstance->OnCombatFinished(bPlayerWon);
	}
}

void UMapManagerSubsystem::GenerateNewStageGraph()
{
	if (!MapGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("MapManager: MapGenerator가 Null입니다!"));
		return;
	}

	//현재 스테이지를 알려주고 맵 그래프 생성을 요청
	GraphRoot = MapGenerator->GenerateStageGraph(this, CurrentStage);
}
