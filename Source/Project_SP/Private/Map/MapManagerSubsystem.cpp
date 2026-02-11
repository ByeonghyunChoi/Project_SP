#include "Map/MapManagerSubsystem.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"


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
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->ResetSaveData();
	}
	bIsReturningFromBattle = false;
	bIsBattleActive = false;
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

void UMapManagerSubsystem::StartBattleEncounter(APawn* PlayerPawn, const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage)
{
	if (!PlayerPawn || !EncounterData) return;

	// 1. [위치 저장] 필드에서의 현재 위치 저장 (돌아올 때를 위해)
	SavedFieldTransform = PlayerPawn->GetActorTransform();
	bIsReturningFromBattle = true;
	bIsBattleActive = true;

	// 2. [스탯 저장] SaveSubsystem에게 플레이어 정보 저장 위임
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->SavePlayerStats(PlayerPawn);
	}

	// 3. [전투 정보] CombatSubsystem 설정
	if (USPCombatSubsystem* CombatSys = GetGameInstance()->GetSubsystem<USPCombatSubsystem>())
	{
		CombatSys->SetPendingEncounter(EncounterData, Advantage);
	}

	// 4. [이동] 레벨 전환 (OpenLevel)
	if (!EncounterData->CombatLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(GetWorld(), EncounterData->CombatLevelName);
		UE_LOG(LogTemp, Log, TEXT("⚔️ 전투 맵으로 이동합니다: %s"), *EncounterData->CombatLevelName.ToString());
	}
}

void UMapManagerSubsystem::ReturnToField()
{
	bIsBattleActive = false;

	LoadStageLevel();
}

void UMapManagerSubsystem::MoveToNextFloor(EMapType SelectedType)
{
	// 다음 스테이지로 넘어가는 경우 (레벨 자체를 갈아타야 함)
	if (CurrentFloor >= 11 && CurrentStage < 3)
	{
		CurrentStage++;
		CurrentFloor = 1;
		bIsReturningFromBattle = false;
		if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
			{
				SaveSys->SavePlayerStats(Player);
			}
		}
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

void UMapManagerSubsystem::SpawnMapActor(EMapType MapType)
{
	//맵 타입을 저장
	CurrentMapType = MapType;

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
			// [CASE A] 전투에서 돌아온 경우
			if (bIsReturningFromBattle)
			{
				UE_LOG(LogTemp, Log, TEXT("🔙 전투 종료! 저장된 위치로 복귀합니다."));

				// 1. 저장된 위치로 이동
				// * ETeleportType::ResetPhysics: 떨어지던 가속도 등을 초기화해서 안전하게 착지
				Player->SetActorTransform(SavedFieldTransform, false, nullptr, ETeleportType::ResetPhysics);

				// 2. 플래그 OFF (다음엔 스폰 포인트로 가야 하니까)
				bIsReturningFromBattle = false;
			}
			// [CASE B] 새로 층에 진입한 경우 (기존 로직)
			else
			{
				UE_LOG(LogTemp, Log, TEXT("🆕 새 층 진입! 스폰 포인트로 이동합니다."));

				TArray<FTransform> Spawns = CurrentMapActor->GetSpawnTransformsByTag(TEXT("SpawnPoint.Player"));
				if (Spawns.Num() > 0)
				{
					Player->SetActorTransform(Spawns[0], false, nullptr, ETeleportType::ResetPhysics);
				}
			}

			// 물리 엔진 관성 초기화 (공통 안전장치)
			if (auto* MoveComp = Player->FindComponentByClass<UCharacterMovementComponent>())
			{
				MoveComp->StopMovementImmediately();
			}
		}

		// 맵 초기화
		CurrentMapActor->InitializeMap(MapType);
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
		// [CASE A] 전투에서 돌아온 경우 -> 저장했던 맵 타입으로 복구
		if (bIsReturningFromBattle)
		{
			// ★ [수정] 무조건 NormalBattle이 아니라, 아까 저장해둔 타입으로 스폰
			SpawnMapActor(CurrentMapType);

			UE_LOG(LogTemp, Log, TEXT("⚔️ Returned from Battle! Restoring Map Type: %d (Floor %d)"), (int32)CurrentMapType, CurrentFloor);

			// 주의: bIsReturningFromBattle = false; 는 SpawnMapActor 안에서 위치 이동 후 처리하므로 여기선 놔둠
		}
		// [CASE B] 새로 스테이지에 진입한 경우 -> 1층, 일반 전투로 시작
		else
		{
			CurrentFloor = 1;
			// 스테이지 첫 진입은 보통 일반 전투
			SpawnMapActor(EMapType::NormalBattle);

			UE_LOG(LogTemp, Warning, TEXT("Stage Level Loaded! Starting New Run (Floor 1)."));
		}
	}
}