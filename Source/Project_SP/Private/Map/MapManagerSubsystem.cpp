#include "Map/MapManagerSubsystem.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Character/SPGASPlayerCharacter.h"


void UMapManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 데이터 테이블 로드
	FString DTPath = TEXT("/Script/Engine.DataTable'/Game/DataTable/DT_MapLevelData.DT_MapLevelData'");
	MapDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DTPath));

	if (!MapDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("FATAL: MapDataTable Load Failed! Path: %s"), *DTPath);
	}

	// 레벨 로드 완료 델리게이트 등록
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMapManagerSubsystem::OnPostLoadMapWithWorld);

	// 로비 레벨 경로
	LobbyLevelReference = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Script/Engine.World'/Game/Field/GameLevel/ReadyMap01.ReadyMap01'")));
}

void UMapManagerSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

void UMapManagerSubsystem::StartNewRun()
{
	USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>();
	APawn* LobbyPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (SaveSys)
	{
		SaveSys->ResetRunData();
		//맵이 넘어가기 전, 로비 플레이어의 '오파츠 장착 및 강화 상태'를 영구 데이터에 덮어쓰기!
		if (LobbyPlayer)
		{
			SaveSys->CachePermDataFromPlayer(LobbyPlayer);
			SaveSys->SavePermToDisk();
			SaveSys->CacheRunDataFromPlayer(LobbyPlayer);
			SaveSys->SaveRunToDisk();

			UE_LOG(LogTemp, Warning, TEXT("필드 진입 전: 로비에서 세팅한 오파츠 데이터를 저장했습니다!"));
		}
	}

	//맵 진행도 초기화
	bIsReturningFromBattle = false;
	bIsBattleActive = false;
	CurrentRoomState = EMapState::InProgress;
	CurrentPortalOptions.Empty();
	CurrentStage = 1;
	CurrentFloor = 1;
	CurrentMapType = EMapType::NormalBattle;
	bIsInLobby = false;

	OnMapLocationChanged.Broadcast(CurrentStage, CurrentFloor);

	LoadStageLevel();
}


void UMapManagerSubsystem::StartBattleEncounter(APawn* PlayerPawn, const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage)
{
	if (!PlayerPawn || !EncounterData) return;

	if (EncounterData->CombatLevelName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("전투 레벨 이름이 없습니다!"));
		return;
	}

	// 필드에서의 현재 위치 저장
	SavedFieldTransform = PlayerPawn->GetActorTransform();
	bIsReturningFromBattle = false;
	bIsBattleActive = true;

	// 플레이어 정보 저장
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->CacheRunDataFromPlayer(PlayerPawn);
		SaveSys->SaveRunToDisk();

		SaveSys->CachePermDataFromPlayer(PlayerPawn);
		SaveSys->SavePermToDisk();
	}

	// 2. [전투 정보] CombatSubsystem 설정 (데이터 전달)
	if (USPCombatSubsystem* CombatSys = GetGameInstance()->GetSubsystem<USPCombatSubsystem>())
	{
		CombatSys->SetPendingEncounter(EncounterData, Advantage);
	}

	// 3. [이동] 전투 레벨로 전환
	UGameplayStatics::OpenLevel(GetWorld(), EncounterData->CombatLevelName);
	UE_LOG(LogTemp, Log, TEXT("전투 맵으로 이동: %s"), *EncounterData->CombatLevelName.ToString());
}

void UMapManagerSubsystem::ReturnToField(bool bIsVictory)
{
	bIsBattleActive = false;
	bIsReturningFromBattle = true;
	CurrentRoomState = bIsVictory ? EMapState::Reward : EMapState::InProgress;

	// 스테이지 레벨 로드 (-> OnPostLoadMapWithWorld가 호출됨)
	LoadStageLevel();

	UE_LOG(LogTemp, Log, TEXT("필드로 복귀합니다."));
}

void UMapManagerSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	if (!MapDataTable) return;

	if (bIsBattleActive)
	{
		UE_LOG(LogTemp, Log, TEXT("전투 레벨 로드 완료 - 맵 스폰을 건너뜁니다."));
		return;
	}

	if (bIsReturningFromBattle || bIsLoadingSave)
	{
		// 로비로 돌아온 게 아니라면 맵을 스폰합니다!
		if (!bIsInLobby)
		{
			SpawnMapActor(CurrentMapType);
			UE_LOG(LogTemp, Log, TEXT("전투/세이브 복귀 완료! (Floor %d)"), CurrentFloor);
		}
	}
	// 2. 처음으로(로비에서) 새 런을 시작하거나, 다음 층으로 넘어온 경우
	else if (!bIsInLobby && !CurrentMapActor.IsValid())
	{
		SpawnMapActor(CurrentMapType);
		UE_LOG(LogTemp, Log, TEXT("새 스테이지/층 진입! (Floor %d)"), CurrentFloor);
	}
}

void UMapManagerSubsystem::SpawnMapActor(EMapType MapType)
{
	CurrentMapType = MapType;

	// 1. 기존 맵 제거
	if (CurrentMapActor.IsValid())
	{
		CurrentMapActor->Destroy();
	}
	CurrentMapActor = nullptr;

	// 2. 맵 액터 스폰
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("SpawnMap"));

	if (Data && Data->MapClasses.Contains(MapType))
	{
		TSubclassOf<AMapBase> MapClassToSpawn = Data->MapClasses[MapType];
		if (MapClassToSpawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			CurrentMapActor = GetWorld()->SpawnActor<AMapBase>(MapClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}

	// 3. 플레이어 위치 설정
	if (CurrentMapActor.IsValid())
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player)
		{
			if (bIsReturningFromBattle || bIsLoadingSave)
			{
				Player->SetActorTransform(SavedFieldTransform, false, nullptr, ETeleportType::ResetPhysics);
			}
			else
			{
				// 새 진입 (포탈 타고 넘어옴)
				TArray<FTransform> Spawns = CurrentMapActor->GetSpawnTransformsByTag(TEXT("SpawnPoint.Player"));
				if (Spawns.Num() > 0)
				{
					Player->SetActorTransform(Spawns[0], false, nullptr, ETeleportType::ResetPhysics);
				}
			}

			// 물리 관성 초기화
			if (auto* MoveComp = Player->FindComponentByClass<UCharacterMovementComponent>())
			{
				MoveComp->StopMovementImmediately();
			}

			if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
			{
				// 1. 오파츠 및 영구 강화 스탯 적용 (새로 스폰될 때마다 필수!)
				SaveSys->RestorePermDataToPlayer(Player);

				// 2. 현재 체력 및 런 중에 먹은 유물 적용
				SaveSys->RestoreRunDataToPlayer(Player);

				if (ASPGASPlayerCharacter* SPPlayer = Cast<ASPGASPlayerCharacter>(Player))
				{
					SPPlayer->CheckLevelUp();
				}

				// 3. 타이틀 화면에서 '이어하기'로 들어온 게 아니라면, 
				// 방금 오파츠까지 싹 입은 완전체 상태를 1층 진입 데이터로 오토세이브!
				if (!bIsLoadingSave)
				{
					GenerateNextFloorOptions();
					SaveSys->CacheRunDataFromPlayer(Player);
					SaveSys->SaveRunToDisk();
					UE_LOG(LogTemp, Warning, TEXT("[오토세이브] 맵 진입 완료 (Stage %d - Floor %d)"), CurrentStage, CurrentFloor);
				}
			}

			// 플래그 초기화
			bIsReturningFromBattle = false;
			bIsLoadingSave = false;
		}

		CurrentMapActor->InitializeMap(MapType, CurrentRoomState);
	}
}

void UMapManagerSubsystem::LoadStageLevel()
{
	bIsInLobby = false;
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("StageLoad"));

	if (Data)
	{
		// 🌟 4층(보스)이면 보스 레벨을, 아니면 일반 레벨을 선택합니다!
		TSoftObjectPtr<UWorld> LevelToLoad = (CurrentFloor >= 4) ? Data->BossLevelReference : Data->NormalLevelReference;

		// (안전장치) 만약 데이터 테이블에 보스 레벨을 안 채워뒀다면 오류 방지를 위해 일반 레벨을 엽니다.
		if (CurrentFloor >= 4 && LevelToLoad.IsNull())
		{
			LevelToLoad = Data->NormalLevelReference;
			UE_LOG(LogTemp, Warning, TEXT("보스 레벨이 비어있어 일반 레벨로 대체합니다!"));
		}

		UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToLoad);
	}
}

void UMapManagerSubsystem::MoveToNextFloor(EMapType SelectedType)
{
	CurrentRoomState = EMapState::InProgress;
	CurrentPortalOptions.Empty();

	if (CurrentFloor >= 4 && CurrentStage < 3)
	{
		// 보스 클리어 후 -> 다음 스테이지 1층으로!
		CurrentStage++;
		CurrentFloor = 1;
		CurrentMapType = SelectedType; // 🌟 새로 들어갈 맵 타입 저장
		bIsReturningFromBattle = false;
		LoadStageLevel(); // 1층이므로 NormalLevelReference가 열립니다.
	}
	else if (CurrentFloor < 4)
	{
		CurrentFloor++;
		CurrentMapType = SelectedType; // 🌟 맵 타입 미리 저장

		// 🌟 방금 올라간 층이 4층(보스)이라면? 맵 액터만 바꾸지 말고 아예 보스 맵(.umap)을 새로 로드!
		if (CurrentFloor == 4)
		{
			bIsReturningFromBattle = false;
			LoadStageLevel(); // 4층이므로 BossLevelReference가 열립니다.
		}
		else
		{
			// 2층, 3층은 물리적인 레벨(.umap) 이동 없이, 같은 레벨 안에서 맵 액터만 갈아 끼웁니다.
			SpawnMapActor(SelectedType);
		}
	}
	else if (CurrentFloor >= 4 && CurrentStage >= 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("🎉 모든 스테이지 클리어! 데모 종료 및 로비로 귀환합니다."));
		GoToLobby();
		return;
	}

	OnMapLocationChanged.Broadcast(CurrentStage, CurrentFloor);
}

EMapGrade UMapManagerSubsystem::GetMapGradeByFloor(int32 Floor) const
{
	switch (Floor)
	{
	case 1: return EMapGrade::Normal;  // 1층: 일반
	case 2: return EMapGrade::Normal;  // 2층: 일반 
	case 3: return EMapGrade::Prepare; // 3층: 준비
	case 4: return EMapGrade::Boss;    // 4층: 보스
	default: return EMapGrade::Normal;
	}
}

EMapType UMapManagerSubsystem::GetRandomTypeFromGrade(EMapGrade Grade) const
{
	switch (Grade)
	{
	case EMapGrade::Normal: return (FMath::RandRange(0, 100) < 80) ? EMapType::NormalBattle : EMapType::Rest;
	case EMapGrade::Epic: return (FMath::RandBool()) ? EMapType::StrongEnemyBattle : EMapType::Jester;
	case EMapGrade::Prepare: return EMapType::Prepare;
	case EMapGrade::Boss: return EMapType::BossBattle;
	default: return EMapType::NormalBattle;
	}
}

TArray<EMapType> UMapManagerSubsystem::GenerateNextFloorOptions()
{
	if (CurrentPortalOptions.Num() > 0)
	{
		return CurrentPortalOptions;
	}

	int32 NextFloor = CurrentFloor + 1;

	// 스테이지의 끝(4층)을 넘어가면, 다음 스테이지 1층의 옵션을 줘야 합니다.
	if (NextFloor > 4)
	{
		CurrentPortalOptions.Add(GetRandomTypeFromGrade(EMapGrade::Normal));
		CurrentPortalOptions.Add(GetRandomTypeFromGrade(EMapGrade::Normal));
		return CurrentPortalOptions;
	}

	EMapGrade NextGrade = GetMapGradeByFloor(NextFloor);

	CurrentPortalOptions.Add(GetRandomTypeFromGrade(NextGrade));
	CurrentPortalOptions.Add(GetRandomTypeFromGrade(NextGrade));

	return CurrentPortalOptions;
}

void UMapManagerSubsystem::ResumeRunFromSave(int32 SavedStage, int32 SavedFloor, EMapType SavedMapType, EMapState SavedRoomState, FTransform SavedTransform, bool bSavedInLobby, TArray<EMapType> SavedPortalOptions)
{
	// 세이브 데이터로 맵 매니저 상태 덮어쓰기
	CurrentStage = SavedStage;
	CurrentFloor = SavedFloor;
	CurrentMapType = SavedMapType;
	CurrentRoomState = SavedRoomState;
	SavedFieldTransform = SavedTransform; // 저장되었던 플레이어 위치!

	// 플래그 세팅
	bIsLoadingSave = true;
	bIsReturningFromBattle = false;
	bIsBattleActive = false;
	bIsInLobby = bSavedInLobby;
	CurrentPortalOptions = SavedPortalOptions;

	// 스테이지 레벨을 열면 -> OnPostLoadMapWithWorld가 작동하면서 맵을 복구함!
	if (bIsInLobby)
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevelReference);
		UE_LOG(LogTemp, Log, TEXT("이어하기: 로비(Lobby) 맵으로 복귀합니다."));
	}
	else
	{
		LoadStageLevel();
		UE_LOG(LogTemp, Log, TEXT("이어하기: 스테이지(Stage) 맵으로 복귀합니다."));
	}
	OnMapLocationChanged.Broadcast(CurrentStage, CurrentFloor);
}

void UMapManagerSubsystem::GoToLobby()
{
	bIsInLobby = true;
	bIsBattleActive = false;
	bIsReturningFromBattle = false;
	CurrentRoomState = EMapState::None;
	// 로비로 돌아오면 런 데이터 초기화
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->ResetRunData();
	}
	if (!LobbyLevelReference.IsNull()) UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevelReference);
}

void UMapManagerSubsystem::InitializeCurrentMap(AMapBase* InMapActor)
{
	// MapBase가 BeginPlay에서 호출해줌
	if (!CurrentMapActor.IsValid()) CurrentMapActor = InMapActor;
}