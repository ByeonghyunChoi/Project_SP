// Subsystems/BattleTransitionManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BattleTransitionManagerSubsystem.generated.h"

class APlayerCharacter;
class UMonsterGroupObject;
class UUserWidget;
class ACombatPawn;

UCLASS(Blueprintable, BlueprintType, Config = Game)
class PROJECT_SP_API UBattleTransitionManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UBattleTransitionManagerSubsystem();
	void RequestEnterBattle(APlayerCharacter* Player, UMonsterGroupObject* MonsterGroup);
	void RequestExitBattle(bool bPlayerWon);
	void NotifyBattleReady(const TArray<ACombatPawn*>& PlayerParty, const TArray<ACombatPawn*>& EnemyParty);

protected:
	FVector LastFieldLocation;
	UPROPERTY() 
	TObjectPtr<APlayerCharacter> PlayerCharacterRef;
	UPROPERTY() 
	TObjectPtr<UMonsterGroupObject> MonsterGroupToBattle;
	UPROPERTY() 
	TObjectPtr<UUserWidget> TransitionWidgetInstance;
	UPROPERTY() 
	TArray<TObjectPtr<ACombatPawn>> CachedPlayerParty;
	UPROPERTY() 
	TArray<TObjectPtr<ACombatPawn>> CachedEnemyParty;

	TSubclassOf<UUserWidget> TransitionWidgetClass;

	UPROPERTY(Config)
	FName BattleArenaMapName;
	UPROPERTY(Config)
	FName BattleStageDirectorTag;

	bool bPlayerWonLastBattle = false;

private:
	bool bLevelStreamingComplete;
	bool bAllPreparationsComplete;
	FTimerHandle LevelStreamingCheckTimer;

	void StartLoadingBattleMap();

	// FadeIn 애니메이션이 끝났을 때 호출될 콜백 함수
	UFUNCTION()
	void OnFadeInAnimationFinished();

	// 레벨 로딩이 완료되었는지 0.1초마다 확인할 함수
	UFUNCTION()
	void CheckLevelStreamingStatus();

	// 레벨 로딩이 확인된 후 호출될 함수
	void OnBattleArenaConfirmed();

	void CheckAndFinalizeTransition();
	void FinalizeBattleStart();
	void UnloadBattleMap();

	UFUNCTION()
	void OnBattleArenaUnloaded();
};