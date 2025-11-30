#include "Combat/BattleTransitionManagerSubsystem.h"
#include "Animation/UMGSequencePlayer.h"
#include "Engine/LevelStreaming.h"
#include "Character/PlayerCharacter.h"
#include "Core/BattleManager.h"
#include "Combat/BattleStageDirector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Character/MyPlayerController.h"
#include "Character/MonsterCharacter.h"
#include "Combat/MonsterGroupObject.h"
#include "Map/MapManagerSubsystem.h"
#include "Misc/OutputDeviceNull.h"

UBattleTransitionManagerSubsystem::UBattleTransitionManagerSubsystem()
{
	// Config 값 초기화 (혹시 ini 설정이 안 되어 있을 경우 대비)
	BattleArenaMapName = TEXT("/Game/Maps/L_BattleArena");
	BattleStageDirectorTag = TEXT("BattleStageDirector");

	static ConstructorHelpers::FClassFinder<UUserWidget> TransitionWidgetRef(TEXT("/Game/Battle/HUD/WBP_BattleTransition.WBP_BattleTransition_C"));
	if (TransitionWidgetRef.Succeeded())
	{
		TransitionWidgetClass = TransitionWidgetRef.Class;
	}
}

void UBattleTransitionManagerSubsystem::RequestEnterBattle(APlayerCharacter* Player, UMonsterGroupObject* MonsterGroup)
{
	bAllPreparationsComplete = false;
	PlayerCharacterRef = Player;
	MonsterGroupToBattle = MonsterGroup;

	if (!PlayerCharacterRef) return;

	// 입력 차단 및 이동 정지
	if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PlayerCharacterRef->GetController()))
	{
		MyPC->SetEnemyTurnInputMode(); // 입력 막기용
	}
	LastFieldLocation = PlayerCharacterRef->GetActorLocation();
	PlayerCharacterRef->GetCharacterMovement()->StopMovementImmediately();

	// Fade In 연출 시작
	if (TransitionWidgetClass)
	{
		TransitionWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TransitionWidgetClass);
		if (TransitionWidgetInstance)
		{
			TransitionWidgetInstance->AddToViewport(100);

			// BP 함수 호출 (PlayFadeIn)
			FOutputDeviceNull Ar;
			TransitionWidgetInstance->CallFunctionByNameWithArguments(TEXT("PlayFadeIn"), Ar, nullptr, true);

			// 애니메이션 시간 후 로딩 시작
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBattleTransitionManagerSubsystem::OnFadeInAnimationFinished, 1.0f, false);
			return;
		}
	}

	// 위젯 없으면 바로 시작
	StartLoadingBattleMap();
}

void UBattleTransitionManagerSubsystem::OnFadeInAnimationFinished()
{
	StartLoadingBattleMap();
}

void UBattleTransitionManagerSubsystem::StartLoadingBattleMap()
{
	// 플레이어 숨김 (필드에서 안 보이게)
	if (PlayerCharacterRef)
	{
		PlayerCharacterRef->SetActorHiddenInGame(true);
		PlayerCharacterRef->SetActorEnableCollision(false);
	}

	// [핵심] 전투 맵을 특정 위치(지하)에 인스턴스로 로드
	bool bSuccess = false;
	CurrentBattleLevelInstance = ULevelStreamingDynamic::LoadLevelInstance(
		this,
		BattleArenaMapName.ToString(),
		BattleMapSpawnLocation,
		BattleMapSpawnRotation,
		bSuccess
	);

	if (bSuccess && CurrentBattleLevelInstance)
	{
		// 로딩 완료(Shown) 델리게이트 연결
		CurrentBattleLevelInstance->OnLevelShown.AddDynamic(this, &UBattleTransitionManagerSubsystem::OnBattleLevelShown);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Battle Map Load Failed! Path: %s"), *BattleArenaMapName.ToString());
		// 실패 시 복구 로직 필요 (여기선 생략)
	}
}

void UBattleTransitionManagerSubsystem::OnBattleLevelShown()
{
	// 델리게이트 해제
	if (CurrentBattleLevelInstance)
	{
		CurrentBattleLevelInstance->OnLevelShown.RemoveDynamic(this, &UBattleTransitionManagerSubsystem::OnBattleLevelShown);
	}

	UE_LOG(LogTemp, Log, TEXT("[Battle] Map Loaded at %s"), *BattleMapSpawnLocation.ToString());

	// Director 찾기 (태그 사용)
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), BattleStageDirectorTag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		if (ABattleStageDirector* StageDirector = Cast<ABattleStageDirector>(FoundActors[0]))
		{
			// StageDirector는 이미 지하 5000m로 이동된 상태로 로드됨
			StageDirector->PrepareBattleScene(PlayerCharacterRef, MonsterGroupToBattle);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: BattleStageDirector Not Found!"));
	}
}

void UBattleTransitionManagerSubsystem::NotifyBattleReady(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
	bAllPreparationsComplete = true;
	CachedPlayerParty = PlayerParty;
	CachedEnemyParty = EnemyParty;

	CheckAndFinalizeTransition();
}

void UBattleTransitionManagerSubsystem::CheckAndFinalizeTransition()
{
	if (bAllPreparationsComplete)
	{
		// Fade Out 연출
		if (TransitionWidgetInstance)
		{
			FOutputDeviceNull Ar;
			TransitionWidgetInstance->CallFunctionByNameWithArguments(TEXT("PlayFadeOut"), Ar, nullptr, true);
		}

		// 화면이 밝아지는 시간(1초) 뒤에 실제 전투 시작
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBattleTransitionManagerSubsystem::FinalizeBattleStart, 1.0f, false);
	}
}

void UBattleTransitionManagerSubsystem::FinalizeBattleStart()
{
	// 위젯 제거
	if (TransitionWidgetInstance)
	{
		TransitionWidgetInstance->RemoveFromParent();
		TransitionWidgetInstance = nullptr;
	}

	// HUD 표시 및 입력 모드 변경
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
		{
			MyPC->ShowBattleHUD();
		}
	}

	// BattleManager 시작
	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		BattleManager->StartBattle(CachedPlayerParty, CachedEnemyParty);
	}
}

void UBattleTransitionManagerSubsystem::RequestExitBattle(bool bPlayerWon)
{
	bPlayerWonLastBattle = bPlayerWon;
	UnloadBattleMap();
}

void UBattleTransitionManagerSubsystem::UnloadBattleMap()
{
	if (PlayerCharacterRef)
	{
		PlayerCharacterRef->SetActorHiddenInGame(true);
	}

	// 전투 맵 언로드
	if (CurrentBattleLevelInstance)
	{
		CurrentBattleLevelInstance->OnLevelHidden.AddDynamic(this, &UBattleTransitionManagerSubsystem::OnBattleLevelHidden);
		CurrentBattleLevelInstance->SetShouldBeLoaded(false);
		CurrentBattleLevelInstance->SetShouldBeVisible(false);
	}
	else
	{
		OnBattleLevelHidden();
	}
}

void UBattleTransitionManagerSubsystem::OnBattleLevelHidden()
{
	if (CurrentBattleLevelInstance)
	{
		CurrentBattleLevelInstance->OnLevelHidden.RemoveDynamic(this, &UBattleTransitionManagerSubsystem::OnBattleLevelHidden);
		CurrentBattleLevelInstance = nullptr;
	}

	// MapManager에게 결과 통보 (보상 등 처리)
	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		MapManager->NotifyCombatFinished(bPlayerWonLastBattle);
	}

	// 플레이어 필드 복귀
	if (PlayerCharacterRef)
	{
		PlayerCharacterRef->SetActorLocation(LastFieldLocation);
		PlayerCharacterRef->SetActorHiddenInGame(false);
		PlayerCharacterRef->SetActorEnableCollision(true);
		PlayerCharacterRef->OnEnterFieldMode();
	}

	// 입력 모드 및 카메라 복구
	if (UWorld* World = GetWorld())
	{
		if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(World->GetFirstPlayerController()))
		{
			MyPC->SetFieldInputMode();
		}
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetViewTargetWithBlend(PlayerCharacterRef.Get(), 0.0f);
	}

	CachedPlayerParty.Empty();
	CachedEnemyParty.Empty();
	MonsterGroupToBattle = nullptr;
}