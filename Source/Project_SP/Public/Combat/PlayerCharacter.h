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

	// 레벨업 처리를 위한 함수
	void LevelUp();

	// 최대 레벨
	const int32 iMaxLevel = 50;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Stats")
	int32 iCurrentEXP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Stats")
	int32 iNextLevelEXP;

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

	//실제 스킬 사용 로직
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void PlayerConfirmSelectedAction();
	//현재 타겟 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player|Combat")
	TArray<ACombatPawn*> SelectedTargetList;

	//타겟 바꾸는 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void PlayerSwitchTarget(bool bSwitchToNext);

	//경험치를 획득하는 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Actions")
	void GainEXP(int32 GainedEXP);
};