// Combat/BattleStageDirector.cpp

#include "Combat/BattleStageDirector.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Component/CombatCameraComponent.h"
#include "Combat/BattleTransitionManagerSubsystem.h"
#include "Character/PlayerCharacter.h"
#include "Character/MonsterCharacter.h"
#include "Combat/MonsterGroupObject.h"
#include "Data/MonsterData.h"
#include "Core/BattleManager.h"
#include "Component/AttributesComponent.h"
#include "Component/ActionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"

ABattleStageDirector::ABattleStageDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ABattleStageDirector::PrepareBattleScene(APlayerCharacter* Player, UMonsterGroupObject* MonsterGroup)
{
	UE_LOG(LogTemp, Error, TEXT("[FLOW 4] BattleStageDirector received request. Preparing scene and requesting asset load..."));
	if (!Player || !MonsterGroup)
	{
		return;
	}
	TempPlayerRef = Player;
	TempMonsterGroupRef = MonsterGroup;

	TArray<FMonsterData> MonstersToSpawn = MonsterGroup->GetAllMonsterDataInGroup();

	TArray<FSoftObjectPath> AssetsToLoad;
	for (const FMonsterData& MonsterData : MonstersToSpawn)
	{
		for (const TSoftObjectPtr<UObject>& AssetPtr : MonsterData.CriticalAssets)
		{
			AssetsToLoad.Add(AssetPtr.ToSoftObjectPath());
		}
	}

	if (AssetsToLoad.Num() > 0)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		FStreamableManager& StreamableManager = AssetManager.GetStreamableManager();

		FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ABattleStageDirector::OnAssetsLoaded);

		StreamableManager.RequestAsyncLoad(AssetsToLoad, OnAssetsLoadedDelegate);
	}
	else
	{
		OnAssetsLoaded();
	}
}

void ABattleStageDirector::OnAssetsLoaded()
{
	UE_LOG(LogTemp, Error, TEXT("[FLOW 5] All critical assets have been loaded. Spawning characters and notifying manager..."));

	APlayerCharacter* Player = TempPlayerRef;
	UMonsterGroupObject* MonsterGroup = TempMonsterGroupRef;
	if (!Player || !MonsterGroup) return;

	Player->SetActorTransform(GetActorTransform());
	Player->SetHomeTransform(GetActorTransform());
	Player->SetActorHiddenInGame(false);
	Player->SetActorEnableCollision(true);
	Player->OnEnterBattleMode();

	TArray<ACombatPawn*> PlayerParty = { Player };
	TArray<ACombatPawn*> EnemyParty = SpawnEnemies(MonsterGroup, Player);

	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		// 2. BattleManager의 카메라 컴포넌트를 가져와 카메라를 설정하고 연출을 재생합니다.
		if (UCombatCameraComponent* CameraComp = BattleManager->GetCameraComponent())
		{
			CameraComp->InitializeCamera(TEXT("Battle_Default_Cam"));
			if (EnemyParty.Num() > 0)
			{
				CameraComp->PlayDefaultShot(Player, EnemyParty[0]);
			}
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBattleTransitionManagerSubsystem* TransitionManager = GameInstance->GetSubsystem<UBattleTransitionManagerSubsystem>())
		{
			UE_LOG(LogTemp, Error, TEXT("[BSD LOG 4] SUCCESS! Notifying BattleTransitionManager that all is ready!"));
			TransitionManager->NotifyBattleReady(PlayerParty, EnemyParty);
		}
	}
}

TArray<ACombatPawn*> ABattleStageDirector::SpawnEnemies(UMonsterGroupObject* MonsterGroup, APlayerCharacter* PlayerToFace)
{
	TArray<ACombatPawn*> SpawnedEnemies;
	TArray<FMonsterData> MonstersToSpawn = MonsterGroup->GetAllMonsterDataInGroup();

	// --- 예전 코드를 기반으로 한 위치 계산 로직 ---
	const FVector SpawnOrigin = GetActorLocation();
	const int32 TotalMonsters = MonstersToSpawn.Num();
	const float CenterIndex = (TotalMonsters - 1) / 2.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = GetLevel();

	for (int32 i = 0; i < TotalMonsters; ++i)
	{
		const float Y_Offset = (i - CenterIndex) * SideSpacing;
		const float X_Offset = BaseForwardDistance + (i - CenterIndex) * DepthSpacing;

		FVector SpawnLocation = SpawnOrigin
			+ GetActorForwardVector() * X_Offset
			+ GetActorRightVector() * Y_Offset;

		// 스폰 시 항상 플레이어를 바라보도록 회전 값 계산
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, PlayerToFace->GetActorLocation());

		AMonsterCharacter* SpawnedMonster = GetWorld()->SpawnActor<AMonsterCharacter>(MonstersToSpawn[i].MonsterClass, SpawnLocation, LookAtRotation, SpawnParams);

		if (SpawnedMonster)
		{
			SpawnedMonster->SetHomeTransform(SpawnedMonster->GetActorTransform());
			// --- 이하 몬스터 초기화 로직은 기존과 동일합니다 ---
			if (UAttributesComponent* AttrComp = SpawnedMonster->GetAttributesComponent())
			{
				AttrComp->SetCharacterID(MonstersToSpawn[i].CharacterStatsRowName);
				AttrComp->InitializeAttributes();
			}
			if (UActionComponent* ActionComp = SpawnedMonster->GetActionComponent())
			{
				ActionComp->InitializeDefaultActions(MonstersToSpawn[i].AvailableActionIDs);
			}
			SpawnedMonster->SetWeaknessType(MonstersToSpawn[i].WeaknessType);
			SpawnedMonster->SetCharacterDisplayName(MonstersToSpawn[i].DisplayName);

			SpawnedEnemies.Add(SpawnedMonster);
		}
	}
	return SpawnedEnemies;
}