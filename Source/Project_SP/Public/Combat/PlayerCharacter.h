#pragma once

#include "CoreMinimal.h"
#include "GameMode/FieldModeComponent.h"
#include "CombatPawn.h"
#include "Data/ActionData.h"
#include "PlayerCharacter.generated.h"

class ABattleManager;
class UWeaponSystemComponent;

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
	// 무기 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponSystemComponent* WeaponSystemComponent;

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

	// 현재 선택된 타겟 (UI에서 표시할 수 있도록)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player|Combat")
	ACombatPawn* CurrentlySelectedTarget;

	//타겟 바꾸는 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void PlayerSwitchTarget(bool bSwitchToNext);
};