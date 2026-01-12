//#include "Map/MapManagerSubsystem.h"
//#include "Map/MapNode.h"
//#include "Map/MapBase.h"
//#include "Map/MapGraphGenerator.h"
//#include "Kismet/GameplayStatics.h"
//#include "Character/SPGASPlayerCharacter.h"
//#include "UObject/ConstructorHelpers.h"
//#include "TimerManager.h"
//#include "Engine/TargetPoint.h"
//#include "Misc/OutputDeviceNull.h" // 위젯 함수 호출용
//
//void UMapManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//
//	// 데이터 테이블 및 위젯 클래스 로드
//	const FString DataTablePath = TEXT("/Game/DataTable/DT_MapData.DT_MapData");
//	MapTypeData = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath));
//
//	const FString WidgetPath = TEXT("/Game/Battle/HUD/WBP_BattleTransition.WBP_BattleTransition_C");
//	TransitionWidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, *WidgetPath);
//	if (!TransitionWidgetClass)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Initialize: 위젯 클래스를 찾을 수 없습니다! 경로를 확인하세요: %s"), *WidgetPath);
//	}
//
//	MapGenerator = NewObject<UMapGraphGenerator>(this);
//}
//
////void UMapManagerSubsystem::StartNewRun()
////{
////	// 새 게임 시작: 1스테이지 그래프 생성 후 첫 노드로 이동
////	CurrentStage = 1;
////	CurrentNode = nullptr;
////	CurrentMapLogicActor = nullptr;
////
////	VisitedNodes.Empty();
////
////	GenerateNewStageGraph();
////	TravelToNode(GraphRoot);
////
////	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
////
////	if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
////	{
////		MyPC->InitStageUI();
////	}
////}
//
//void UMapManagerSubsystem::TravelToNode(UMapNode* TargetNode)
//{
//	if (!TargetNode) return;
//	PendingNode = TargetNode;
//
//	VisitedNodes.Add(TargetNode);
//
//	// [Step 1] 로딩 시작 전 화면을 검게 가림 (Fade In)
//	if (TransitionWidgetClass)
//	{
//		if (!CurrentTransitionWidget)
//		{
//			CurrentTransitionWidget = CreateWidget<UUserWidget>(GetWorld(), TransitionWidgetClass);
//		}
//
//		if (CurrentTransitionWidget)
//		{
//			CurrentTransitionWidget->AddToViewport(9999);
//
//			// BP 함수 'PlayFadeIn' 호출
//			FOutputDeviceNull Ar;
//			CurrentTransitionWidget->CallFunctionByNameWithArguments(TEXT("PlayFadeIn"), Ar, nullptr, true);
//
//			// 애니메이션 시간(1초) 후 로딩 프로세스 시작
//			FTimerHandle TimerHandle;
//			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMapManagerSubsystem::OnFadeInFinished, 1.0f, false);
//			return;
//		}
//	}
//
//	// 위젯 없으면 바로 로딩 (비상용)
//	OnFadeInFinished();
//}
//
//void UMapManagerSubsystem::OnFadeInFinished()
//{
//	// 화면이 가려졌으니 기존 맵을 언로드하거나 새 맵을 로드합니다.
//	if (CurrentLevelInstance)
//	{
//		UnloadPreviousLevel();
//	}
//	else
//	{
//		LoadNextLevel();
//	}
//}
//
//void UMapManagerSubsystem::UnloadPreviousLevel()
//{
//	if (!IsValid(CurrentLevelInstance))
//	{
//		OnLevelUnloaded();
//		return;
//	}
//
//	// 2. [핵심 수정] 이미 숨겨져 있는 상태라면, 델리게이트가 안 불릴 수 있음 -> 즉시 완료 처리
//	// (이 체크가 없으면 영원히 대기하거나 로직이 꼬일 수 있음)
//	if (!CurrentLevelInstance->IsLevelVisible())
//	{
//		// 메모리에는 올라와 있지만 눈에는 안 보이는 상태 -> 그냥 바로 날려버림
//		CurrentLevelInstance->SetShouldBeLoaded(false);
//		OnLevelUnloaded();
//		return;
//	}
//
//	// 3. 정상적인 언로드 절차
//	// 숨겨짐(Hidden) 상태가 되면 OnLevelUnloaded 함수를 호출해달라고 등록
//	CurrentLevelInstance->OnLevelHidden.AddDynamic(this, &UMapManagerSubsystem::OnLevelUnloaded);
//
//	// 보이지 않게 하고(Visible=false), 메모리에서 내림(Loaded=false)
//	CurrentLevelInstance->SetShouldBeVisible(false);
//	CurrentLevelInstance->SetShouldBeLoaded(false);
//}
//
//void UMapManagerSubsystem::OnLevelUnloaded()
//{
//	// 델리게이트 해제 및 포인터 초기화
//	if (CurrentLevelInstance)
//	{
//		CurrentLevelInstance->OnLevelHidden.RemoveDynamic(this, &UMapManagerSubsystem::OnLevelUnloaded);
//		CurrentLevelInstance = nullptr;
//	}
//
//	if (CurrentMapLogicActor)
//	{
//		CurrentMapLogicActor->Destroy(); // 액터를 월드에서 제거
//		CurrentMapLogicActor = nullptr;  // 포인터 초기화
//	}
//
//	// 깨끗해졌으니 다음 레벨 로드
//	LoadNextLevel();
//}
//
//void UMapManagerSubsystem::LoadNextLevel()
//{
//	if (!PendingNode || !MapTypeData) return;
//
//	// 데이터 테이블에서 맵 타입에 맞는 정보(레벨 경로) 가져오기
//	const FName RowName = UEnum::GetValueAsName(PendingNode->MapType);
//	FMapDataRow* Row = MapTypeData->FindRow<FMapDataRow>(RowName, TEXT(""));
//
//	if (!Row || Row->LevelAsset.IsNull())
//	{
//		UE_LOG(LogTemp, Error, TEXT("Level Asset Not Found!"));
//		PerformFadeOut(); // 에러 나도 화면은 밝혀줘야 함
//		return;
//	}
//
//	FVector SpawnLocation = FVector(-15000.0f, 0.0f, 0.0f);
//
//	// [핵심] 레벨 인스턴스 비동기 로드
//	bool bSuccess = false;
//	CurrentLevelInstance = ULevelStreamingDynamic::LoadLevelInstance(
//		this,
//		Row->LevelAsset.GetLongPackageName(),
//		SpawnLocation, // 위치 (필요 시 변경 가능)
//		FRotator::ZeroRotator,
//		bSuccess
//	);
//
//	if (bSuccess && CurrentLevelInstance)
//	{
//		// 로딩 완료 시점(Shown)을 잡기 위해 델리게이트 연결
//		CurrentLevelInstance->OnLevelShown.AddDynamic(this, &UMapManagerSubsystem::OnLevelLoaded);
//	}
//}
//
////void UMapManagerSubsystem::OnLevelLoaded()
////{
////	// 1. 델리게이트 해제
////	if (CurrentLevelInstance)
////	{
////		CurrentLevelInstance->OnLevelShown.RemoveDynamic(this, &UMapManagerSubsystem::OnLevelLoaded);
////	}
////
////	// 2. 맵 로직 액터(BP) 동적 스폰
////	const FName RowName = UEnum::GetValueAsName(PendingNode->MapType);
////	FMapDataRow* Row = MapTypeData->FindRow<FMapDataRow>(RowName, TEXT(""));
////
////	if (Row && Row->MapLogicClass)
////	{
////		FActorSpawnParameters SpawnParams;
////		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
////
////		// 데이터 테이블에 지정된 BP 클래스(BP_Map_Normal 등)를 스폰
////		CurrentMapLogicActor = GetWorld()->SpawnActor<AMapBase>(
////			Row->MapLogicClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
////	}
////
////	// 3. 맵 초기화 및 플레이어 이동
////	if (CurrentMapLogicActor)
////	{
////		// 현재 노드 정보 갱신
////		CurrentNode = PendingNode;
////
////		// 다음 갈 수 있는 곳 정보 전달
////		CurrentMapLogicActor->InitializeNextNodes(CurrentNode->ChildNodes);
////
////		// 플레이어를 레벨 내 시작 지점(PlayerStartPoint 태그)으로 이동
////		if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
////		{
////			// MapBase가 태그를 검색해서 위치를 알려줌
////			Player->SetActorLocationAndRotation(
////				CurrentMapLogicActor->GetPlayerStartLocation(),
////				CurrentMapLogicActor->GetPlayerStartRotation()
////			);
////		}
////
////		// [중요] 맵 로직 시작 (몬스터/포탈 스폰 등)
////		CurrentMapLogicActor->BeginMapLogic();
////	}
////
////	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
////	if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
////	{
////		MyPC->UpdateStageUI(); // <- 여기서 화살표 위치 갱신!
////	}
////
////	// 4. 모든 준비 완료 -> 화면 밝히기
////	PerformFadeOut();
////}
//
//void UMapManagerSubsystem::PerformFadeOut()
//{
//	if (CurrentTransitionWidget)
//	{
//		FOutputDeviceNull Ar;
//		CurrentTransitionWidget->CallFunctionByNameWithArguments(TEXT("PlayFadeOut"), Ar, nullptr, true);
//
//		// 애니메이션 후 위젯 제거
//		FTimerHandle TimerHandle;
//		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
//			{
//				if (CurrentTransitionWidget)
//				{
//					CurrentTransitionWidget->RemoveFromParent();
//					CurrentTransitionWidget = nullptr;
//				}
//			}, 1.0f, false);
//	}
//}
//
//void UMapManagerSubsystem::NotifyCombatFinished(bool bPlayerWon)
//{
//	if (CurrentMapLogicActor)
//	{
//		CurrentMapLogicActor->OnCombatFinished(bPlayerWon);
//	}
//}
//
//void UMapManagerSubsystem::GoToNextStage()
//{
//	CurrentStage++;
//	if (CurrentStage > MaxStages)
//	{
//		ReturnToHub(true);
//		return;
//	}
//	GenerateNewStageGraph();
//	TravelToNode(GraphRoot);
//}
//
//void UMapManagerSubsystem::ReturnToHub(bool bPlayerWon)
//{
//	// 맵 정리 및 허브 이동 로직 (기존 유지)
//	if (CurrentLevelInstance)
//	{
//		CurrentLevelInstance->SetShouldBeLoaded(false);
//		CurrentLevelInstance->SetShouldBeVisible(false);
//		CurrentLevelInstance = nullptr;
//	}
//	CurrentMapLogicActor = nullptr;
//	CurrentNode = nullptr;
//
//	// 허브 스폰 포인트로 이동
//	TArray<AActor*> FoundActors;
//	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), HubSpawnPointTag, FoundActors);
//	if (FoundActors.Num() > 0)
//	{
//		if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
//		{
//			Player->SetActorLocationAndRotation(FoundActors[0]->GetActorLocation(), FoundActors[0]->GetActorRotation());
//		}
//	}
//}
//
////void UMapManagerSubsystem::GetCurrentStageLayout(TArray<EMapType>& OutMapTypes, int32& OutCurrentIndex)
////{
////	OutMapTypes.Empty();
////	const int32 TotalLayers = 10; // 0~9층
////
////	// 1. [과거] 이미 방문한 노드는 실제 데이터를 넣습니다.
////	for (UMapNode* Node : VisitedNodes)
////	{
////		if (Node) OutMapTypes.Add(Node->MapType);
////	}
////
////	// 현재 플레이어 위치 (배열 인덱스 기준)
////	OutCurrentIndex = VisitedNodes.Num() - 1;
////
////	// 2. [미래] 아직 안 간 곳은 '규칙'에 따라 대표 타입을 넣습니다.
////	// MapGraphGenerator의 생성 규칙과 일치시켜야 합니다.
////	for (int32 i = VisitedNodes.Num(); i < TotalLayers; ++i)
////	{
////		EMapType PredictedType = EMapType::NormalBattle;
////
////		switch (i)
////		{
////			// [에픽 구간] 2층(3번째), 5층(6번째) -> 강적 or 광대지만 '강적'으로 통일
////		case 2:
////		case 5:
////			PredictedType = EMapType::StrongEnemyBattle;
////			break;
////
////			// [준비 구간] 8층
////		case 8:
////			PredictedType = EMapType::Prepare;
////			break;
////
////			// [보스 구간] 9층
////		case 9:
////			PredictedType = EMapType::BossBattle;
////			break;
////
////			// [일반 구간] 나머지 -> 일반 or 휴식이지만 '일반'으로 통일
////		default:
////			PredictedType = EMapType::NormalBattle;
////			break;
////		}
////		OutMapTypes.Add(PredictedType);
////	}
////}
//
//void UMapManagerSubsystem::GenerateNewStageGraph()
//{
//	if (MapGenerator)
//	{
//		GraphRoot = MapGenerator->GenerateStageGraph(this, CurrentStage);
//	}
//}