//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "Map/MapBase.h"
//#include "Components/SceneComponent.h"
//#include "Map/MapNode.h"
//#include "Map/PortalActor.h"
//#include "Map/RewardBox.h"
//#include "Kismet/GameplayStatics.h"
//#include"SubSystem/SoundManagerSubsystem.h"
//
//// Sets default values
//AMapBase::AMapBase()
//{
//	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot")));
//	/*CurrentMapState = EMapState::InProgress;
//	CurrentMapType = EMapType::NormalBattle;*/
//}
//
//void AMapBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
//{
//	Super::EndPlay(EndPlayReason);
//
//	// 맵 관리자가 사라질 때, 관리하던 모든 액터도 같이 정리합니다.
//	ClearMapElements();
//}
//
//void AMapBase::BeginPlay()
//{
//	Super::BeginPlay();
//
//	if (USoundManagerSubsystem* SoundMgr = GetGameInstance()->GetSubsystem<USoundManagerSubsystem>())
//	{
//		// StageBGM이 nullptr이면 아무 소리도 안 남 (의도된 정적 가능)
//		SoundMgr->PlayFieldBGM(StageBGM);
//	}
//}
//
//
//void AMapBase::OnRewardBoxOpened()
//{
//	ActivatePortals();
//}
//
////FName AMapBase::GetRewardRowNameByMapType() const
////{
////	switch (CurrentMapType)
////    {
////    case EMapType::NormalBattle:
////        return FName("Normal"); // 일반 전투 보상
////
////    case EMapType::StrongEnemyBattle:
////        return FName("Epic");  // 강적 전투 보상
////
////    case EMapType::BossBattle:
////        return FName("Boss");   // 보스 전투 보상
////
////    case EMapType::Jester:
////        return FName("Epic");   // 이벤트 맵 보상
////
////    default:
////        return FName("Normal");
////    }
////}
//
//void AMapBase::BeginMapLogic_Implementation()
//{
//	// 1. 플레이어 시작 위치 찾기 (캐싱)
//	if (!LevelPlayerStartActor)
//	{
//		TArray<AActor*> FoundActors;
//		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("PlayerStartPoint"), FoundActors);
//		if (FoundActors.Num() > 0)
//		{
//			LevelPlayerStartActor = FoundActors[0];
//		}
//	}
//
//	// 2. 포탈 초기화 (비활성화 상태로)
//	// (자식 BP에서 포탈을 스폰한 뒤 이 함수가 호출되어야 함)
//	InitPortalsToInactive();
//}
//
//void AMapBase::OnCombatFinished_Implementation(bool bPlayerWon)
//{
//	if (bPlayerWon)
//	{
//		//SetMapState(EMapState::Cleard);
//
//		// 보상 상자 스폰 및 초기화
//		SpawnRewardBox();
//	}
//}
//
//void AMapBase::SpawnRewardBox()
//{
//	// 만약 자식 BP에서 이미 스폰하고 변수에 할당했다면 이 로직이 실행됨
//	if (RewardBox)
//	{
//		// 보상 데이터 초기화
//		FName TargetLootGroup = GetRewardRowNameByMapType();
//		RewardBox->InitializeReward(RewardDataTable, TargetLootGroup, RelicDataTable);
//
//		// [핵심] 상호작용 이벤트 연결 (상자 열면 -> OnRewardBoxOpened 호출)
//		RewardBox->OnRewardInteracted.AddDynamic(this, &AMapBase::OnRewardBoxOpened);
//	}
//}
//
//void AMapBase::InitializeNextNodes(const TArray<UMapNode*>& ChildNodes)
//{
//	NextNodeOptions = ChildNodes;
//}
//
//void AMapBase::ClearMapElements()
//{
//	for (APortalActor* Portal : PortalActors)
//	{
//		if (Portal)
//		{
//			Portal->Destroy();
//		}
//	}
//	PortalActors.Empty();
//}
//
//void AMapBase::ActivatePortals()
//{
//	for (APortalActor* Portal : PortalActors)
//	{
//		if (Portal)
//		{
//			// 열린 상태로 전환
//			Portal->SetActorEnableCollision(true);
//			Portal->OnPortalStateChanged(true);
//		}
//	}
//}
//
//void AMapBase::InitPortalsToInactive()
//{
//	int32 NumToInit = FMath::Min(NextNodeOptions.Num(), PortalActors.Num());
//	for (int32 i = 0; i < NumToInit; ++i)
//	{
//		if (APortalActor* Portal = PortalActors[i])
//		{
//			Portal->InitializePortalData(NextNodeOptions[i]);
//
//			// 닫힌 상태로 시작
//			Portal->SetActorEnableCollision(false);
//			Portal->OnPortalStateChanged(false);
//		}
//	}
//}
//
////void AMapBase::SetMapType(const EMapType& NewMapType)
////{
////	CurrentMapType = NewMapType;
////}
////
////EMapType AMapBase::GetMapType() const
////{
////	return CurrentMapType;
////}
////
////void AMapBase::SetMapState(const EMapState& NewMapState)
////{
////	CurrentMapState = NewMapState;
////}
////
////EMapState AMapBase::GetMapState() const
////{
////	return CurrentMapState;
////}
//
//FVector AMapBase::GetPlayerStartLocation() const
//{
//	if (LevelPlayerStartActor) return LevelPlayerStartActor->GetActorLocation();
//
//	TArray<AActor*> FoundActors;
//	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("PlayerStartPoint"), FoundActors);
//	if (FoundActors.Num() > 0) return FoundActors[0]->GetActorLocation();
//
//	return GetActorLocation();
//}
//
//FRotator AMapBase::GetPlayerStartRotation() const
//{
//	if (LevelPlayerStartActor) return LevelPlayerStartActor->GetActorRotation();
//
//	TArray<AActor*> FoundActors;
//	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("PlayerStartPoint"), FoundActors);
//	if (FoundActors.Num() > 0) return FoundActors[0]->GetActorRotation();
//
//	return GetActorRotation();
//}
//
