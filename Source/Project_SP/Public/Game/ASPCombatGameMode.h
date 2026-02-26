// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASPCombatGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API AASPCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AASPCombatGameMode();
	virtual void BeginPlay() override;

	void StartTurn(AActor* TurnActor);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndTurn(AActor* TurnActor);

protected:
	// 블루프린트에서 BP_TurnManager를 지정해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Battle")
	TSubclassOf<class ASPCombatTurnManager> TurnManagerClass;

	// 실제 생성된 매니저
	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	TObjectPtr <class ASPCombatTurnManager> TurnManager;

	// 현재 턴 주인
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Battle")
	TObjectPtr<AActor> CurrentTurnActor;

	// 전투 참가자 목록
	UPROPERTY()
	TArray<AActor*> AllParticipants;

	UPROPERTY(EditDefaultsOnly, Category = "Battle|TimePower")
	TSubclassOf<class UGameplayEffect> TurnEndTimeCostGE;

private:
	FTransform GetSpawnTransformByIndex(int32 Index);

	void InitializeBattle(const TArray<AActor*>& Enemies, APawn* Player);
};
