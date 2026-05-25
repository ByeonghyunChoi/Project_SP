#include "Map/MapManagerSubsystem.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Data/Asset/SPStageMonsterPoolData.h"


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

	FString PoolPath = TEXT("/Script/Project_SP.SPStageMonsterPoolData'/Game/DataTable/DataAsset/MonsterPoolData.MonsterPoolData'");

	StagePoolDataAsset = Cast<USPStageMonsterPoolData>(StaticLoadObject(USPStageMonsterPoolData::StaticClass(), nullptr, *PoolPath));

	{
		UE_LOG(LogTemp, Error, TEXT("FATAL: StagePoolDataAsset Load Failed! Path: %s"), *PoolPath);
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

	PreGenerateAllEncounters();

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

	if (bIsVictory)
	{
		// 현재 열려있는 맵(레벨)의 이름을 문자열로 가져옴
		FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		ClearedStageName = FName(*CurrentLevel); // FName으로 변환해서 저장

		UE_LOG(LogTemp, Warning, TEXT("자동 메모 작성 완료! 클리어한 맵: %s"), *CurrentLevel);
	}
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

	if (CurrentFloor >= 5 && CurrentStage < 2) // 5층(보스) 클리어 시
	{
		CurrentStage++;
		CurrentFloor = 1;
		CurrentMapType = SelectedType;
		bIsReturningFromBattle = false;
		LoadStageLevel();
	}
	else if (CurrentFloor < 5)
	{
		CurrentFloor++;
		CurrentMapType = SelectedType;

		if (CurrentFloor == 5) // 방금 올라간 층이 5층이라면 보스맵 로드
		{
			bIsReturningFromBattle = false;
			LoadStageLevel();
		}
		else
		{
			SpawnMapActor(SelectedType);
		}
	}
	else if (CurrentFloor >= 5 && CurrentStage >= 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("🎉 모든 스테이지 클리어! 데모 종료 및 로비로 귀환합니다."));
		GoToLobby();
		return;
	}

	OnMapLocationChanged.Broadcast(CurrentStage, CurrentFloor);
}

EMapType UMapManagerSubsystem::PickAndRemoveWeightedMap(TMap<EMapType, int32>& InOutWeightPool)
{
	if (InOutWeightPool.IsEmpty()) return EMapType::NormalBattle;

	int32 TotalWeight = 0;
	for (const auto& Pair : InOutWeightPool)
	{
		TotalWeight += Pair.Value;
	}

	int32 WinningTicket = FMath::RandRange(1, TotalWeight);

	for (auto It = InOutWeightPool.CreateIterator(); It; ++It)
	{
		WinningTicket -= It->Value;

		// 0 이하가 되면 이 항목이 당첨된 것입니다!
		if (WinningTicket <= 0)
		{
			EMapType SelectedMap = It->Key;

			// 한 번 뽑힌 맵은 다음 추첨을 위해 바구니에서 제거합니다!
			It.RemoveCurrent();

			return SelectedMap;
		}
	}

	return EMapType::NormalBattle;
}

TArray<EMapType> UMapManagerSubsystem::GenerateNextFloorOptions()
{
	if (CurrentPortalOptions.Num() > 0)
	{
		return CurrentPortalOptions;
	}

	int32 NextFloor = CurrentFloor + 1;

	if (NextFloor > 5)
	{
		NextFloor = 1;
	}

	// 4층(준비 맵)일 때 옵션을 딱 1개만 넣습니다!
	if (NextFloor == 4)
	{
		CurrentPortalOptions.Add(EMapType::Prepare);
		return CurrentPortalOptions;
	}

	// 5층(보스 맵)일 때 옵션을 딱 1개만 넣습니다!
	if (NextFloor == 5)
	{
		CurrentPortalOptions.Add(EMapType::BossBattle);
		return CurrentPortalOptions;
	}

	// 그 외 일반 층(2층, 3층)일 때는 정상적으로 가중치를 돌려 2개를 뽑습니다.
	TMap<EMapType, int32> FloorWeightPool;
	FloorWeightPool.Add(EMapType::NormalBattle, 3);
	FloorWeightPool.Add(EMapType::Rest, 1);
	FloorWeightPool.Add(EMapType::StrongEnemyBattle, 1);
	FloorWeightPool.Add(EMapType::Jester, 500);

	CurrentPortalOptions.Add(PickAndRemoveWeightedMap(FloorWeightPool));
	CurrentPortalOptions.Add(PickAndRemoveWeightedMap(FloorWeightPool));

	return CurrentPortalOptions;
}

void UMapManagerSubsystem::PreGenerateAllEncounters()
{
	PreGeneratedEncounters.Empty();

	if (!StagePoolDataAsset) return;

	// 1스테이지부터 3스테이지까지, 각 1층부터 5층까지 전부 미리 뽑습니다!
	for (int32 TargetStage = 1; TargetStage <= 2; ++TargetStage)
	{
		if (!StagePoolDataAsset->StagePools.Contains(TargetStage)) continue;
		const FStageMonsterPool& Pool = StagePoolDataAsset->StagePools[TargetStage];

		for (int32 TargetFloor = 1; TargetFloor <= 5; ++TargetFloor)
		{
			FSavedEncounterData EncounterData;
			EncounterData.CombatLevelName = Pool.CombatLevelName;

			// 해당 층에 맞는 레벨 미리 계산
			int32 MonsterLevel = ((TargetStage - 1) * 10) + ((TargetFloor - 1) * 3) + 1;

			if (Pool.NormalMonsters.Num() > 0)
			{
				FSavedEncounterData NormalEncounter;
				NormalEncounter.CombatLevelName = Pool.CombatLevelName;

				for (int32 i = 0; i < 3; ++i)
				{
					FEnemySpawnInfo Slot;
					int32 RandomIdx = FMath::RandRange(0, Pool.NormalMonsters.Num() - 1);

					Slot.MonsterData = Pool.NormalMonsters[RandomIdx];
					Slot.SpawnLevel = MonsterLevel;
					Slot.SpawnPositionIndex = i; // 0, 1, 2번에 각각 스폰
					Slot.bOverrideWeakness = true;
					Slot.OverriddenWeaknessTags = GenerateRandomWeaknesses(FMath::RandRange(1, 2));

					NormalEncounter.EnemyGroup.Add(Slot);
				}

				// 🌟 [핵심] Key 규칙 변경: (스테이지 * 1000) + (층수 * 10) + 맵타입(Enum)
				int32 NormalKey = (TargetStage * 1000) + (TargetFloor * 10) + (uint8)EMapType::NormalBattle;
				PreGeneratedEncounters.Add(NormalKey, NormalEncounter);
			}

			// ==============================================================
			// 2. 강적 전투 (StrongEnemyBattle) 명부 작성: 1마리 스폰
			// ==============================================================
			if (Pool.StrongMonsters.Num() > 0)
			{
				FSavedEncounterData StrongEncounter;
				StrongEncounter.CombatLevelName = Pool.CombatLevelName;

				FEnemySpawnInfo Slot;
				int32 RandomIdx = FMath::RandRange(0, Pool.StrongMonsters.Num() - 1);

				Slot.MonsterData = Pool.StrongMonsters[RandomIdx];
				Slot.SpawnLevel = MonsterLevel;
				Slot.SpawnPositionIndex = 1;		// 🌟 강적은 중앙(1번)에 위풍당당하게 1마리만 스폰!
				Slot.bOverrideWeakness = true;
				Slot.OverriddenWeaknessTags = GenerateRandomWeaknesses(FMath::RandRange(1, 2));

				StrongEncounter.EnemyGroup.Add(Slot);

				// 🌟 강적 전용 Key 발급
				int32 StrongKey = (TargetStage * 1000) + (TargetFloor * 10) + (uint8)EMapType::StrongEnemyBattle;
				PreGeneratedEncounters.Add(StrongKey, StrongEncounter);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[MapManager] 이번 런의 모든 몬스터 인카운터 명부가 고정되었습니다!"));
}

FRewardResult UMapManagerSubsystem::CalculateCombatRewards(const TArray<EMonsterRank>& DefeatedRanks, int32 Stage, EMapType MapType)
{
	FRewardResult TotalReward;

	// 죽은 몬스터 수만큼 반복하면서 스테이지/맵 타입에 따라 보상을 누적!
	for (EMonsterRank Rank : DefeatedRanks)
	{
		if (Stage == 1)
		{
			TotalReward.Exp += (Rank == EMonsterRank::Boss) ? 250 : 60;
			if (MapType == EMapType::NormalBattle) { TotalReward.Gold += 70; TotalReward.Sand += 10; TotalReward.IncompleteEnergy += 2; }
			else if (MapType == EMapType::StrongEnemyBattle) { TotalReward.Gold += 83; TotalReward.Sand += 13; TotalReward.IncompleteEnergy += 3; }
		}
		else if (Stage == 2)
		{
			TotalReward.Exp += (Rank == EMonsterRank::Boss) ? 1300 : 330;
			if (MapType == EMapType::NormalBattle) { TotalReward.Gold += 80; TotalReward.Sand += 12; TotalReward.IncompleteEnergy += 2; }
			else if (MapType == EMapType::StrongEnemyBattle) { TotalReward.Gold += 96; TotalReward.Sand += 15; TotalReward.IncompleteEnergy += 4; }
		}
		else if (Stage >= 3)
		{
			TotalReward.Exp += (Rank == EMonsterRank::Boss) ? 2500 : 600;
			if (MapType == EMapType::NormalBattle) { TotalReward.Gold += 90; TotalReward.Sand += 14; TotalReward.IncompleteEnergy += 3; }
			else if (MapType == EMapType::StrongEnemyBattle) { TotalReward.Gold += 100; TotalReward.Sand += 17; TotalReward.IncompleteEnergy += 5; }
		}
	}
	return TotalReward;
}

FRewardResult UMapManagerSubsystem::GenerateInteractableReward(bool bIsHealingObject, int32 Stage, EMapType MapType)
{
	FRewardResult Result;

	// 1. 배율 설정 (소수점 반올림을 위해 float 사용)
	float Multiplier = 1.0f;
	if (Stage == 2) Multiplier = 1.15f;
	else if (Stage >= 3) Multiplier = 1.3f;

	// 2. 보스 맵 상자 하드코딩 (고정 보상)
	if (MapType == EMapType::BossBattle)
	{
		if (Stage == 1) { Result.Gold = 800; Result.Sand = 150; Result.IncompleteEnergy = 30; Result.Fragment = 1; Result.Exp = 150; }
		else if (Stage == 2) { Result.Gold = 2000; Result.Sand = 600; Result.IncompleteEnergy = 100; Result.Fragment = 1; Result.Exp = 820; }
		else { Result.Gold = 3000; Result.Sand = 1500; Result.IncompleteEnergy = 250; Result.Fragment = 1; Result.Exp = 1536; }

		Result.RelicRewardCount = 1;
		Result.bIsBossReward = true; 
		return Result;
	}

	// 3. 내부 가중치 랜덤 함수 람다 정의
	auto GetWeightedRandom = [](const TArray<int32>& Weights) -> int32 {
		int32 Total = 0;
		for (int32 W : Weights) Total += W;
		int32 RandNum = FMath::RandRange(1, Total);
		for (int32 i = 0; i < Weights.Num(); ++i) {
			RandNum -= Weights[i];
			if (RandNum <= 0) return i;
		}
		return 0;
		};

	// 4. 상호작용 타입별 가중치 추첨
	if (bIsHealingObject)
	{
		// 0: Gold, 1: Sand, 2: Energy, 3: Relic
		int32 Pick = GetWeightedRandom({ 30, 40, 20, 10 });
		if (Pick == 0) Result.Gold = FMath::RoundToInt(FMath::RandRange(75, 100) * Multiplier);
		else if (Pick == 1) Result.Sand = FMath::RoundToInt(FMath::RandRange(20, 30) * Multiplier);
		else if (Pick == 2) Result.IncompleteEnergy = FMath::RoundToInt(FMath::RandRange(5, 7) * Multiplier);
		else if (Pick == 3) Result.RelicRewardCount = 1;
	}
	else if (MapType == EMapType::StrongEnemyBattle) // 에픽(강적) 맵 상자
	{
		Result.RelicRewardCount = 1; // 🌟 확정 유물 1개 지급

		// 추가 보상 추첨
		// 0: Relic(20), 1: Gold(40), 2: Sand(30), 3: Energy(10), 4: Frag(10)
		int32 Pick = GetWeightedRandom({ 20, 40, 30, 10, 10 });
		if (Pick == 0) Result.RelicRewardCount += 1; // 추가 유물 당첨 (총 2개)
		else if (Pick == 1) Result.Gold = FMath::RoundToInt(FMath::RandRange(350, 450) * Multiplier);
		else if (Pick == 2) Result.Sand = FMath::RoundToInt(FMath::RandRange(100, 150) * Multiplier);
		else if (Pick == 3) Result.IncompleteEnergy = FMath::RoundToInt(FMath::RandRange(25, 40) * Multiplier);
		else if (Pick == 4) Result.Fragment = FMath::RoundToInt(2 * Multiplier); // 파편도 배율을 타나요? 아니라면 배율 제거
	}
	else // 일반 맵 상자
	{
		// 0: Relic(30), 1: Gold(30), 2: Sand(25), 3: Energy(10), 4: Frag(5)
		int32 Pick = GetWeightedRandom({ 30, 30, 25, 10, 5 });
		if (Pick == 0) Result.RelicRewardCount = 1;
		else if (Pick == 1) Result.Gold = FMath::RoundToInt(FMath::RandRange(150, 200) * Multiplier);
		else if (Pick == 2) Result.Sand = FMath::RoundToInt(FMath::RandRange(40, 60) * Multiplier);
		else if (Pick == 3) Result.IncompleteEnergy = FMath::RoundToInt(FMath::RandRange(10, 15) * Multiplier);
		else if (Pick == 4) Result.Fragment = FMath::RoundToInt(1 * Multiplier);
	}

	return Result;
}

int32 UMapManagerSubsystem::CalculateMonsterLevel() const
{
	return ((CurrentStage - 1) * 10) + ((CurrentFloor - 1) * 3) + 1;
}

FGameplayTagContainer UMapManagerSubsystem::GenerateRandomWeaknesses(int32 Count)
{
	FGameplayTagContainer RandomTags;
	const FSPGameplayTags& Tags = FSPGameplayTags::Get();
	TArray<FGameplayTag> ElementPool = { Tags.Weakness_Surtr, Tags.Weakness_Jormungandr, Tags.Weakness_Fenrir };

	// 중복 없이 Count만큼 랜덤 추출
	while (RandomTags.Num() < Count && ElementPool.Num() > 0)
	{
		int32 Idx = FMath::RandRange(0, ElementPool.Num() - 1);
		RandomTags.AddTag(ElementPool[Idx]);
		ElementPool.RemoveAt(Idx);
	}
	return RandomTags;
}

UCombatEncounterData* UMapManagerSubsystem::GenerateFieldEncounter()
{
	int32 Key = (CurrentStage * 1000) + (CurrentFloor * 10) + (uint8)CurrentMapType;

	if (PreGeneratedEncounters.Contains(Key))
	{
		UCombatEncounterData* NewEncounter = NewObject<UCombatEncounterData>(this);
		NewEncounter->CombatLevelName = PreGeneratedEncounters[Key].CombatLevelName;
		NewEncounter->EnemyGroup = PreGeneratedEncounters[Key].EnemyGroup;

		UE_LOG(LogTemp, Warning, TEXT("미리 고정된 몬스터 구성을 불러옵니다. (Stage %d - Floor %d - MapType %d)"),
			CurrentStage, CurrentFloor, (uint8)CurrentMapType);

		return NewEncounter;
	}

	UE_LOG(LogTemp, Error, TEXT("FATAL: 해당 층/맵 타입에 고정된 몬스터 명부가 없습니다! (Key: %d)"), Key);
	return nullptr;
}

FTransform UMapManagerSubsystem::GetFieldSpawnTransform(int32 Index)
{
	TArray<AActor*> FoundActors;
	// FieldSpawn_숫자 태그를 가진 액터 찾기
	FName TargetTag = FName(*FString::Printf(TEXT("FieldSpawn_%d"), Index));
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		return FoundActors[0]->GetActorTransform();
	}

	return FTransform::Identity;
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

	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		PreGeneratedEncounters = SaveSys->GetRunData().MapProgress.PreGeneratedEncounters;
	}

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

EMapGrade UMapManagerSubsystem::GetMapGradeForUI(int32 Floor) const
{
	switch (Floor)
	{
	case 1: return EMapGrade::Normal;
	case 2: return EMapGrade::Normal;
	case 3: return EMapGrade::Normal;
	case 4: return EMapGrade::Prepare;
	case 5: return EMapGrade::Boss;
	default: return EMapGrade::Normal;
	}
}