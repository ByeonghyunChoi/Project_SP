// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterBase.h"
#include "PlayerCharacter.h"
#include "BattleManager.generated.h"

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	None UMETA(DisplayName = "None"),
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
	
public:	
	// Sets default values for this actor's properties
	ABattleManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 턴 순서 결정 로직
	void SortReadyCombatants(TArray<UCharacterBase*>& ReadyCombatants);

	// 전투 종료 조건을 확인하는 함수
	bool CheckBattleEndConditions();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//-------------------------------------변수 부분--------------------------------------
	// 전투의 현재 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	EBattleState CurrentBattleState;

	// 전투가 시작된 이후의 전역 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	float GlobalTime;

	// 현재 전투에 참여하고 있는 모든 캐릭터 (UCharacterBase 인스턴스)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<UCharacterBase*> AllCombatants;

	// 현재 턴인 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	UCharacterBase* CurrentTurnCharacter;


	//--------------------------------------함수 부분-------------------------------------
	//전투 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartBattle(TArray<UCharacterBase*> InitialCombatants);

	// 전투 종료 함수
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndBattle();

	// 턴 진행 로직 (Tick에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ProcessTurn();

	// 턴을 획득할 준비가 된 캐릭터들을 우선순위에 따라 정렬하는 함수
	// 이 함수는 APlayerCharacter, AMonsterCharacter에서 포함된 UCharacterBase 인스턴스를
	// AllCombatants 배열에 추가할 때 사용됩니다.
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void AddCombatant(UCharacterBase* NewCombatant);

};
