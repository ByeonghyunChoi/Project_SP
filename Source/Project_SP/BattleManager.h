// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatPawn.h"
#include "BattleManager.generated.h"

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Setup UMETA(DisplayName = "전투 준비"), 
	InProgress UMETA(DisplayName = "전투 진행 중"), 
	PlayerTurn UMETA(DisplayName = "플레이어 턴"), 
	EnemyTurn UMETA(DisplayName = "적 턴"),   
	Ended UMETA(DisplayName = "전투 종료")             
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnOrderChanged);

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

	// 현재 턴인 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	ACombatPawn* CurrentTurnCharacter;

	// 전투 종료 조건을 확인하는 함수
	bool CheckBattleEndConditions() const;

	// 특정 캐릭터에게 턴을 부여하고 행동을 시작하도록 지시
	void InitiateTurnFor(ACombatPawn* TargetCombatant);

	// 턴을 획득할 준비가 된 캐릭터들을 규칙에 따라 정렬
	void SortReadyCombatants(TArray<ACombatPawn*>& ReadyCombatants) const;

public:
	//생성자
	ABattleManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//턴 로직 호출 부분
	virtual void Tick(float DeltaTime) override;

public:	

	// 현재 전투에 참여하고 있는 모든 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TArray<ACombatPawn*> AllCombatants;

	// 턴 순서 변경 시 UI에 알릴 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Battle Events")
	FOnTurnOrderChanged OnTurnOrderChanged;

	// 전투 시작 함수 (BP_CombatGameMode에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartBattle(TArray<ACombatPawn*> InitialCombatants);

	// 턴 진행의 메인 진입점 (Tick 또는 EndTurn에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ProcessTurn();

	//전투 참여자 추가
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void AddCombatant(ACombatPawn* NewCombatant);

	// 전투 종료 함수
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndBattle();

	// 턴 종료 함수 (UBattleModeComponent 또는 AMonsterCharacter에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndTurn();
};
