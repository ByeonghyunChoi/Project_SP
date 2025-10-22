// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/BattleTransitionManager.h"
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

UBattleTransitionManager::UBattleTransitionManager()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> TransitionWidgetRef(TEXT("/Game/Battle/HUD/WBP_BattleTransition.WBP_BattleTransition_C"));
	if (TransitionWidgetRef.Succeeded())
	{
		TransitionWidgetClass = TransitionWidgetRef.Class;
	}
}

void UBattleTransitionManager::RequestEnterBattle(APlayerCharacter* Player, UMonsterGroupObject* MonsterGroup)
{
	bLevelStreamingComplete = false;
	bAllPreparationsComplete = false;
	PlayerCharacterRef = Player;
	MonsterGroupToBattle = MonsterGroup;
	if (!PlayerCharacterRef) return;

	if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PlayerCharacterRef->GetController()))
	{
		MyPC->SetEnemyTurnInputMode();
	}

	LastFieldLocation = PlayerCharacterRef->GetActorLocation();
	PlayerCharacterRef->GetCharacterMovement()->StopMovementImmediately();

	if (TransitionWidgetClass)
	{
		TransitionWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TransitionWidgetClass);
		if (TransitionWidgetInstance)
		{
			TransitionWidgetInstance->AddToViewport(100);

			UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(TransitionWidgetInstance->GetClass());
			for (UWidgetAnimation* Anim : WidgetClass->Animations)
			{
				// 'FadeIn'으로 시작하는 1회성 애니메이션을 찾습니다.
				if (Anim && Anim->GetFName().ToString().StartsWith(TEXT("FadeIn")))
				{
					FWidgetAnimationDynamicEvent OnAnimFinished;
					OnAnimFinished.BindUFunction(this, FName("OnFadeInAnimationFinished"));
					TransitionWidgetInstance->BindToAnimationFinished(Anim, OnAnimFinished);

					TransitionWidgetInstance->PlayAnimation(Anim);
					return; // 로딩은 애니메이션이 끝난 후 시작됩니다.
				}
			}
		}
	}

	// 위젯이나 애니메이션이 없으면, 즉시 로딩을 시작합니다.
	StartLoadingBattleMap();
}

void UBattleTransitionManager::OnFadeInAnimationFinished()
{
	// FadeIn 애니메이션이 끝났으므로, 안전하게 레벨 로딩을 시작합니다.
	StartLoadingBattleMap();
}

void UBattleTransitionManager::StartLoadingBattleMap()
{
	if (PlayerCharacterRef)
	{
		PlayerCharacterRef->SetActorHiddenInGame(true);
		PlayerCharacterRef->SetActorEnableCollision(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Attempting to load stream level with name: %s"), *BattleArenaMapName.ToString());

	// 로딩을 시작하고, 콜백 없이 바로 다음으로 넘어갑니다.
	UGameplayStatics::LoadStreamLevel(this, BattleArenaMapName, true, true, FLatentActionInfo());

	// 로딩이 끝났는지 0.1초마다 확인하는 타이머를 시작합니다.
	GetWorld()->GetTimerManager().SetTimer(LevelStreamingCheckTimer, this, &UBattleTransitionManager::CheckLevelStreamingStatus, 0.1f, true);
}

void UBattleTransitionManager::CheckLevelStreamingStatus()
{
	ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, BattleArenaMapName);
	if (StreamingLevel && StreamingLevel->IsLevelLoaded() && StreamingLevel->IsLevelVisible())
	{
		GetWorld()->GetTimerManager().ClearTimer(LevelStreamingCheckTimer);
		OnBattleArenaConfirmed();
	}
}


void UBattleTransitionManager::OnBattleArenaConfirmed()
{
	UE_LOG(LogTemp, Error, TEXT("[FLOW 3] Battle Arena level is confirmed to be LOADED and VISIBLE."));
	bLevelStreamingComplete = true;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), BattleStageDirectorTag, FoundActors);
	if (FoundActors.Num() > 0)
	{
		if (ABattleStageDirector* StageDirector = Cast<ABattleStageDirector>(FoundActors[0]))
		{
			StageDirector->PrepareBattleScene(PlayerCharacterRef, MonsterGroupToBattle);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Battle Arena loaded, but NO actor with tag '%s' was found!"), *BattleStageDirectorTag.ToString());
	}
}

void UBattleTransitionManager::NotifyBattleReady(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty)
{
	UE_LOG(LogTemp, Error, TEXT("[FLOW 6] Received notification that battle is ready. Starting final transition..."));
	bAllPreparationsComplete = true;

	CachedPlayerParty = PlayerParty;
	CachedEnemyParty = EnemyParty;

	CheckAndFinalizeTransition();
}

void UBattleTransitionManager::CheckAndFinalizeTransition()
{
	if (bLevelStreamingComplete && bAllPreparationsComplete)
	{
		UE_LOG(LogTemp, Log, TEXT("Battle Readiness: ALL TASKS COMPLETE. Starting fade out."));

		float FadeOutDuration = 1.0f;
		if (TransitionWidgetInstance)
		{
			UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(TransitionWidgetInstance->GetClass());
			for (UWidgetAnimation* Anim : WidgetClass->Animations)
			{
				if (Anim && Anim->GetFName().ToString().StartsWith(TEXT("FadeOut")))
				{
					TransitionWidgetInstance->PlayAnimation(Anim);
					FadeOutDuration = Anim->GetEndTime();
					break;
				}
			}
		}

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBattleTransitionManager::FinalizeBattleStart, FadeOutDuration, false);
	}
}

void UBattleTransitionManager::FinalizeBattleStart()
{
	UE_LOG(LogTemp, Error, TEXT("[FLOW 7] Finalizing... Calling BattleManager->StartBattle() NOW!"));

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
		{
			MyPC->ShowBattleHUD();
		}
	}
	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		BattleManager->StartBattle(CachedPlayerParty, CachedEnemyParty);
	}

	if (TransitionWidgetInstance)
	{
		TransitionWidgetInstance->RemoveFromParent();
		TransitionWidgetInstance = nullptr;
	}
}

void UBattleTransitionManager::RequestExitBattle(bool bPlayerWon)
{
	bPlayerWonLastBattle = bPlayerWon;
	UnloadBattleMap();
}

void UBattleTransitionManager::UnloadBattleMap()
{
	if (PlayerCharacterRef)
	{
		PlayerCharacterRef->SetActorHiddenInGame(true);
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName("OnBattleArenaUnloaded");
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = FMath::Rand();
	UGameplayStatics::UnloadStreamLevel(this, BattleArenaMapName, LatentInfo, false);
}

void UBattleTransitionManager::OnBattleArenaUnloaded()
{
	if (bPlayerWonLastBattle && MonsterGroupToBattle)
	{
		TArray<AActor*> FoundMonsters;
		// 2. 현재 월드(필드)에 있는 모든 몬스터 캐릭터를 찾습니다.
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMonsterCharacter::StaticClass(), FoundMonsters);

		for (AActor* Actor : FoundMonsters)
		{
			AMonsterCharacter* Monster = Cast<AMonsterCharacter>(Actor);
			// 3. 몬스터가 우리가 싸웠던 '그 몬스터 그룹'에 속해있는지 확인합니다.
			if (Monster && Monster->GetCombatMonsterGroup() == MonsterGroupToBattle)
			{
				// 4. 일치하면 필드에서 몬스터를 파괴합니다.
				Monster->Destroy();
			}
		}
	}

	if (PlayerCharacterRef)
	{
		PlayerCharacterRef->SetActorLocation(LastFieldLocation); 
		PlayerCharacterRef->SetActorHiddenInGame(false); 
		PlayerCharacterRef->SetActorEnableCollision(true); 
		PlayerCharacterRef->OnEnterFieldMode(); 
	}
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
}
