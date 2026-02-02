#include "Map/MapManagerSubsystem.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMapManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//여기에 실제 데이터 테이블 경로를 넣으세요!
	FString DTPath = TEXT("/Script/Engine.DataTable'/Game/DataTable/DT_MapLevelData.DT_MapLevelData'");
	MapDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DTPath));

	if (!MapDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("FATAL: MapDataTable Load Failed! Path: %s"), *DTPath);
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMapManagerSubsystem::OnPostLoadMapWithWorld);

	LobbyLevelReference = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Script/Engine.World'/Game/Field/GameLevel/TestMap01_Field.TestMap01_Field'")));
}

void UMapManagerSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

void UMapManagerSubsystem::StartNewRun()
{
	CurrentStage = 1;
	CurrentFloor = 1;
	LoadStageLevel();
}

EMapGrade UMapManagerSubsystem::GetMapGradeByFloor(int32 Floor) const
{
	// 11개 노드 등급 규칙: N-N-E-N-N-E-N-N-E-P-B
	switch (Floor)
	{
	case 1: case 2: case 4: case 5: case 7: case 8: return EMapGrade::Normal;
	case 3: case 6: case 9: return EMapGrade::Epic;
	case 10: return EMapGrade::Prepare;
	case 11: return EMapGrade::Boss;
	default: return EMapGrade::Normal;
	}
}

EMapType UMapManagerSubsystem::GetRandomTypeFromGrade(EMapGrade Grade) const
{
	switch (Grade)
	{
	case EMapGrade::Normal:
		return (FMath::RandRange(0, 100) < 80) ? EMapType::NormalBattle : EMapType::Rest;
	case EMapGrade::Epic:
		return (FMath::RandBool()) ? EMapType::StrongEnemyBattle : EMapType::Jester;
	case EMapGrade::Prepare: return EMapType::Prepare;
	case EMapGrade::Boss: return EMapType::BossBattle;
	default: return EMapType::NormalBattle;
	}
}

TArray<EMapType> UMapManagerSubsystem::GenerateNextFloorOptions()
{
	TArray<EMapType> Options;
	int32 NextFloor = CurrentFloor + 1;

	// 다음 스테이지로 넘어가는 경우 (1-11 클리어 후)
	if (NextFloor > 11) {
		Options.Add(EMapType::NormalBattle); // 다음 스테이지 1-1은 항상 전투
		return Options;
	}

	EMapGrade NextGrade = GetMapGradeByFloor(NextFloor);

	// 선택지 2개 생성
	Options.Add(GetRandomTypeFromGrade(NextGrade));
	Options.Add(GetRandomTypeFromGrade(NextGrade));

	return Options;
}

void UMapManagerSubsystem::MoveToNextFloor(EMapType SelectedType)
{
	// 다음 스테이지로 넘어가는 경우 (레벨 자체를 갈아타야 함)
	if (CurrentFloor >= 11 && CurrentStage < 3)
	{
		CurrentStage++;
		CurrentFloor = 1;
		LoadStageLevel(); // OpenLevel 실행 (로딩 발생)
		return;
	}

	// 같은 스테이지 내에서 층만 이동 (즉시 교체)
	if (CurrentFloor < 11)
	{
		CurrentFloor++;
		SpawnMapActor(SelectedType); // ★ 여기서 맵 갈아끼우기 실행
	}
}

void UMapManagerSubsystem::EnterBattle(TSoftObjectPtr<UWorld> BattleLevelRes)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player || BattleLevelRes.IsNull()) return;

	SavedFieldTransform = Player->GetActorTransform();
	bool bOutSuccess = false;
	ActiveBattleLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(GetWorld(), BattleLevelRes, BattleMapOffset, FRotator::ZeroRotator, bOutSuccess);

	if (bOutSuccess && ActiveBattleLevel)
	{
		ActiveBattleLevel->OnLevelLoaded.AddDynamic(this, &UMapManagerSubsystem::OnBattleLevelLoaded);
	}
}

void UMapManagerSubsystem::OnBattleLevelLoaded()
{
	if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		Player->SetActorLocation(BattleMapOffset + FVector(0, 0, 100));
	}
		
}

void UMapManagerSubsystem::SpawnMapActor(EMapType MapType)
{
	// 1. 기존 맵이 있다면 제거
	if (CurrentMapActor)
	{
		CurrentMapActor->Destroy();
		CurrentMapActor = nullptr;
	}

	// 2. 데이터 테이블에서 현재 스테이지 정보 가져오기
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("SpawnMap"));

	if (Data && Data->MapClasses.Contains(MapType))
	{
		// 3. 해당 타입에 맞는 클래스(BP) 가져오기
		TSubclassOf<AMapBase> MapClassToSpawn = Data->MapClasses[MapType];

		if (MapClassToSpawn)
		{
			// 4. 월드 원점(0,0,0)에 새 맵 스폰
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			CurrentMapActor = GetWorld()->SpawnActor<AMapBase>(MapClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}

	// 5. 플레이어 이동 및 맵 초기화
	if (CurrentMapActor)
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player)
		{
			// 새 맵의 스폰 포인트로 이동
			TArray<FTransform> Spawns = CurrentMapActor->GetSpawnTransformsByTag(TEXT("SpawnPoint.Player"));
			if (Spawns.Num() > 0)
			{
				Player->SetActorTransform(Spawns[0]);
				// 물리 엔진 관성 초기화 (필수)
				if (auto* MoveComp = Player->FindComponentByClass<UCharacterMovementComponent>())
					MoveComp->StopMovementImmediately();
			}
		}

		// 맵 초기화 (몬스터 스폰 등)
		CurrentMapActor->InitializeMap(MapType);
	}
}

void UMapManagerSubsystem::ExitBattle()
{
	if (ActiveBattleLevel)
	{
		ActiveBattleLevel->SetIsRequestingUnloadAndRemoval(true);
		ActiveBattleLevel = nullptr;

		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player) Player->SetActorTransform(SavedFieldTransform);

		if (CurrentMapActor)
		{
			CurrentMapActor->SetActorTickEnabled(true);
			CurrentMapActor->SetMapState(EMapState::Reward);
		}
	}
}

void UMapManagerSubsystem::LoadStageLevel()
{
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("StageLoad"));
	if (Data) UGameplayStatics::OpenLevelBySoftObjectPtr(this, Data->LevelReference);
}

void UMapManagerSubsystem::InitializeCurrentMap(AMapBase* InMapActor)
{
	CurrentMapActor = InMapActor;
	if (CurrentMapActor)
	{
		// 1층이면 무조건 일반 전투, 아니면 층 등급에 따라 랜덤 생성 (혹은 저장된 타입)
		EMapType InitialType = (CurrentFloor == 1) ? EMapType::NormalBattle : GetRandomTypeFromGrade(GetMapGradeByFloor(CurrentFloor));

		UE_LOG(LogTemp, Log, TEXT("Initializing Map: Stage %d - Floor %d (Type: %d)"), CurrentStage, CurrentFloor, (int32)InitialType);

		CurrentMapActor->InitializeMap(InitialType);
	}
}

void UMapManagerSubsystem::GoToLobby()
{
	// 런 정보 초기화
	CurrentStage = 1;
	CurrentFloor = 0; // 0층은 대기 상태
	CurrentMapActor = nullptr; // 기존 맵 액터 참조 끊기

	if (!LobbyLevelReference.IsNull())
	{
		// 로비 레벨 열기
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevelReference);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Lobby Level Reference is NULL!"));
	}
}

void UMapManagerSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// 1. 현재 로드된 레벨이 우리가 의도한 '스테이지 레벨'인지 확인해야 함
	// (메인 메뉴나 엉뚱한 레벨일 수도 있으므로)

	// 안전장치: 데이터 테이블이 없으면 무시
	if (!MapDataTable) return;

	// 2. 현재 스테이지 이름으로 데이터 테이블 검색
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("CheckStage"));

	if (Data && Data->LevelReference.GetAssetName() == LoadedWorld->GetName())
	{
		// 3. 맞다면 1층 맵 스폰 시작! (항상 일반 전투로 시작)
		CurrentFloor = 1;
		SpawnMapActor(EMapType::NormalBattle);

		UE_LOG(LogTemp, Warning, TEXT("Stage Level Loaded! Spawning Floor 1 Map."));
	}
}