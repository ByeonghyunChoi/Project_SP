// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/FieldModeComponent.h"
#include "CombatPawn.h"
#include "Data/ActionData.h"
#include "PlayerCharacter.generated.h"

class ABattleManager;
UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// 필드 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UFieldModeComponent* FieldModeComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// --- 이벤트 핸들러 ---
	// ACombatPawn의 GameEventComponent::OnTurnStarted 이벤트 구독
	UFUNCTION()
	void HandlePlayerTurnStarted(ACombatPawn* TurnPawn);
	// ACombatPawn의 GameEventComponent::OnCombatPawnStateChanged 이벤트 구독
	UFUNCTION()
	void HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState);

public:
	// 모드 전환 함수
	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterFieldMode();

	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterBattleMode();

	// --- 플레이어 행동/타겟 선택 로직 ---
   // UI에서 행동 선택 버튼 클릭 시 호출
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void PlayerSelectAction(FName ActionID); // FName으로 ActionID 받음

	// 타겟팅 모드에서 몬스터를 마우스 좌클릭 시 호출 (임시)
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void PlayerTargetMonsterClicked(ACombatPawn* Target);

	// 타겟 확정 (마우스 좌클릭 또는 UI 버튼)
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void PlayerConfirmTargets(); // 타겟 확정 후 ExecuteConfirmedAction 호출

	// 현재 선택된 타겟 (UI에서 표시할 수 있도록)
	UPROPERTY(BlueprintReadWrite, Category = "Player|Combat")
	ACombatPawn* CurrentlySelectedTarget;
};
