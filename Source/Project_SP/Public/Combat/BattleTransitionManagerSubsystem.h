// Subsystems/BattleTransitionManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/LevelStreamingDynamic.h"
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
	void RequestEnterBattle(APlayerCharacter* Player, UMonsterGroupObject* MonsterGroup, AActor* FieldMonsterActor);
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
	UPROPERTY()
	TObjectPtr<AActor> CachedFieldMonsterActor;

	TSubclassOf<UUserWidget> TransitionWidgetClass;

	UPROPERTY(Config)
	FName BattleArenaMapName;
	UPROPERTY(Config)
	FName BattleStageDirectorTag;

	bool bPlayerWonLastBattle = false;

	// [추가] 로드된 전투 레벨 인스턴스
	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> CurrentBattleLevelInstance;

	// [추가] 전투 맵이 스폰될 위치 (필드와 겹치지 않게 지하로 설정)
	FVector BattleMapSpawnLocation = FVector(0.0f, 0.0f, -5000.0f);
	FRotator BattleMapSpawnRotation = FRotator::ZeroRotator;

private:
	bool bAllPreparationsComplete;

	// 내부 로직 순서
	void StartLoadingBattleMap();

	UFUNCTION()
	void OnFadeInAnimationFinished();

	// [변경] 델리게이트 콜백 함수들
	UFUNCTION()
	void OnBattleLevelShown();

	UFUNCTION()
	void OnBattleLevelHidden();

	void CheckAndFinalizeTransition();
	void FinalizeBattleStart();
	void UnloadBattleMap();
};