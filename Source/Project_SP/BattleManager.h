// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatPawn.h"
#include "BattleManager.generated.h"

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Setup UMETA(DisplayName = "Setup"), 
	InProgress UMETA(DisplayName = "In Progress"), 
	PlayerTurn UMETA(DisplayName = "Player Turn"), 
	EnemyTurn UMETA(DisplayName = "Enemy Turn"),   
	Ended UMETA(DisplayName = "Ended")             
};

UCLASS()
class PROJECT_SP_API ABattleManager : public AActor
{
	GENERATED_BODY()
	
private:
	// 전투의 현재 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	EBattleState CurrentBattleState;

	// 전투가 시작된 이후의 전역 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	float GlobalTime;

	// 현재 전투에 참여하고 있는 모든 캐릭터 (UCharacterBase 인스턴스)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TArray<ACombatPawn*> AllCombatants;

	// 현재 턴인 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	ACombatPawn* CurrentTurnCharacter;

	//---------------------------------함수------------------------------------
	// 턴 진행을 위한 내부 함수
	void AdvanceTimeAndFindNextTurn();

	// 전투 종료 조건을 확인하는 함수
	bool CheckBattleEndConditions();

	// 턴 순서 결정 로직
	void SortReadyCombatants(TArray<ACombatPawn*>& ReadyCombatants) const;

	// 특정 캐릭터에게 턴을 부여하고 행동을 시작하도록 지시
	void InitiateTurnFor(ACombatPawn* TargetCombatant);


public:
	//생성자
	ABattleManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//턴 로직 호출 부분
	virtual void Tick(float DeltaTime) override;

public:	
	// 전투 시작 함수 
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartBattle(TArray<ACombatPawn*> InitialCombatants);

	// 전투 종료 함수
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndBattle();

	// 턴 진행의 메인 진입점 (Tick, EndTurn에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ProcessTurn();

	// 전투 참여자 추가
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void AddCombatant(ACombatPawn* NewCombatant);

	// 턴 종료 함수 (UBattleModeComponent 또는 AMonsterCharacter에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndTurn();

	// 현재 턴을 가질 수 있는 모든 전투 참여자들의 배열을 반환 (정렬된 상태)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	TArray<ACombatPawn*> GetTurnOrderCombatants() const;

};
