#include "Map/MapManagerSubsystem.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMapManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MapDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("")));

	if (!MapDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("MapManagerSubsystem: Failed to load MapDataTable!"));
	}
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
	if (CurrentFloor < 11)
	{
		CurrentFloor++;
		APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player && CurrentMapActor)
		{
			TArray<FTransform> Spawns = CurrentMapActor->GetSpawnTransformsByTag(TEXT("SpawnPoint.Player"));
			if (Spawns.Num() > 0) Player->SetActorTransform(Spawns[0]);
			if (auto* MoveComp = Player->FindComponentByClass<UCharacterMovementComponent>())
				MoveComp->StopMovementImmediately();
		}
		CurrentMapActor->InitializeMap(SelectedType);
	}
	else if (CurrentStage < 3)
	{
		CurrentStage++; CurrentFloor = 1;
		LoadStageLevel();
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
		EMapType InitialType = (CurrentFloor == 1) ? EMapType::NormalBattle : GetRandomTypeFromGrade(GetMapGradeByFloor(CurrentFloor));
		CurrentMapActor->InitializeMap(InitialType);
	}
}