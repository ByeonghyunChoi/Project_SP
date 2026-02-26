#include "Map/MapManagerSubsystem.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"


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
	LobbyLevelReference = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Script/Engine.World'/Game/Field/GameLevel/TestMap01_Field.TestMap01_Field'")));
}

void UMapManagerSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

void UMapManagerSubsystem::StartNewRun()
{
	// 새 게임 시작 시 데이터 초기화
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->ResetSaveData();
	}

	bIsReturningFromBattle = false;
	bIsBattleActive = false;
	CurrentStage = 1;
	CurrentFloor = 1;
	CurrentMapType = EMapType::NormalBattle;

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

	// 1. [위치 저장] 필드에서의 현재 위치 저장
	SavedFieldTransform = PlayerPawn->GetActorTransform();
	bIsReturningFromBattle = true;
	bIsBattleActive = true;

	// 2. [스탯 저장] SaveSubsystem에게 플레이어 정보 저장 위임
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		SaveSys->SavePlayerStats(PlayerPawn);
	}

	// 3. [전투 정보] CombatSubsystem 설정 (데이터 전달)
	if (USPCombatSubsystem* CombatSys = GetGameInstance()->GetSubsystem<USPCombatSubsystem>())
	{
		CombatSys->SetPendingEncounter(EncounterData, Advantage);
	}

	// 4. [이동] 전투 레벨로 전환
	UGameplayStatics::OpenLevel(GetWorld(), EncounterData->CombatLevelName);
	UE_LOG(LogTemp, Log, TEXT("전투 맵으로 이동: %s"), *EncounterData->CombatLevelName.ToString());
}

void UMapManagerSubsystem::ReturnToField()
{
	bIsBattleActive = false;

	// 스테이지 레벨 로드 (-> OnPostLoadMapWithWorld가 호출됨)
	LoadStageLevel();

	UE_LOG(LogTemp, Log, TEXT("필드로 복귀합니다."));
}

void UMapManagerSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	if (!MapDataTable) return;

	// 현재 로드된 레벨이 '스테이지 레벨'인지 확인
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("CheckStage"));

	if (Data && Data->LevelReference.GetAssetName() == LoadedWorld->GetName())
	{
		// [CASE A] 전투에서 돌아온 경우 -> 기존 맵 복구 및 플레이어 위치 이동
		if (bIsReturningFromBattle)
		{
			SpawnMapActor(CurrentMapType);
			UE_LOG(LogTemp, Log, TEXT("전투 복귀 완료! 저장된 위치로 이동합니다. (Floor %d)"), CurrentFloor);
		}
		// [CASE B] 게임 시작 / 스테이지 이동 / 로드 게임
		else
		{
			// 맵 액터가 없으면 새로 생성 (첫 진입)
			if (!CurrentMapActor)
			{
				SpawnMapActor(CurrentMapType);
				UE_LOG(LogTemp, Log, TEXT("새 스테이지 진입! (Floor %d)"), CurrentFloor);
			}
		}
	}
}

void UMapManagerSubsystem::SpawnMapActor(EMapType MapType)
{
	CurrentMapType = MapType;

	// 1. 기존 맵 제거
	if (CurrentMapActor)
	{
		CurrentMapActor->Destroy();
		CurrentMapActor = nullptr;
	}

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
	if (CurrentMapActor)
	{
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player)
		{
			if (bIsReturningFromBattle)
			{
				// 전투 복귀: 저장된 위치로
				Player->SetActorTransform(SavedFieldTransform, false, nullptr, ETeleportType::ResetPhysics);
				bIsReturningFromBattle = false; // 플래그 초기화
			}
			else
			{
				// 새 진입: 스폰 포인트로
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
		}

		CurrentMapActor->InitializeMap(MapType);
	}
}

void UMapManagerSubsystem::LoadStageLevel()
{
	FString RowName = FString::Printf(TEXT("Stage%d"), CurrentStage);
	FMapLevelData* Data = MapDataTable->FindRow<FMapLevelData>(FName(*RowName), TEXT("StageLoad"));
	if (Data) UGameplayStatics::OpenLevelBySoftObjectPtr(this, Data->LevelReference);
}

void UMapManagerSubsystem::MoveToNextFloor(EMapType SelectedType)
{
	if (CurrentFloor >= 11 && CurrentStage < 3)
	{
		// 다음 스테이지로
		CurrentStage++;
		CurrentFloor = 1;
		bIsReturningFromBattle = false;
		LoadStageLevel();
	}
	else if (CurrentFloor < 11)
	{
		// 같은 스테이지 다음 층
		CurrentFloor++;
		SpawnMapActor(SelectedType);
	}
}

EMapGrade UMapManagerSubsystem::GetMapGradeByFloor(int32 Floor) const
{
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
	case EMapGrade::Normal: return (FMath::RandRange(0, 100) < 80) ? EMapType::NormalBattle : EMapType::Rest;
	case EMapGrade::Epic: return (FMath::RandBool()) ? EMapType::StrongEnemyBattle : EMapType::Jester;
	case EMapGrade::Prepare: return EMapType::Prepare;
	case EMapGrade::Boss: return EMapType::BossBattle;
	default: return EMapType::NormalBattle;
	}
}

TArray<EMapType> UMapManagerSubsystem::GenerateNextFloorOptions()
{
	TArray<EMapType> Options;
	int32 NextFloor = CurrentFloor + 1;
	if (NextFloor > 11) { Options.Add(EMapType::NormalBattle); return Options; }
	EMapGrade NextGrade = GetMapGradeByFloor(NextFloor);
	Options.Add(GetRandomTypeFromGrade(NextGrade));
	Options.Add(GetRandomTypeFromGrade(NextGrade));
	return Options;
}

void UMapManagerSubsystem::GoToLobby()
{
	StartNewRun(); // 데이터 초기화
	if (!LobbyLevelReference.IsNull()) UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevelReference);
}

void UMapManagerSubsystem::InitializeCurrentMap(AMapBase* InMapActor)
{
	// MapBase가 BeginPlay에서 호출해줌
	if (!CurrentMapActor) CurrentMapActor = InMapActor;
}