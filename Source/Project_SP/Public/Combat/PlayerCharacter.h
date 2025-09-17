#pragma once

#include "CoreMinimal.h"
#include "GameMode/FieldModeComponent.h"
#include "CombatPawn.h"
#include "Data/ActionData.h"
#include "PlayerCharacter.generated.h"

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
	TObjectPtr<UWeaponSystemComponent> WeaponSystemComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// --- 이벤트 핸들러 ---
	// ACombatPawn의 GameEventComponent::OnTurnStarted 이벤트 구독
	UFUNCTION()
	void HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState);

	//현재 타겟
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat")
	TArray<TObjectPtr<ACombatPawn>> CurrentTargets;


public:
	virtual void OnTurnBegin() override;

	// 모드 전환 함수
	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterFieldMode();

	UFUNCTION(BlueprintCallable, Category = "Modes")
	void EnterBattleMode();

	//무기 교체 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Input")
	void RequestSwitchWeapon(EDamageType WeaponType);

	//지정된 행동 실행 시도 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Input")
	void RequestStartAction(FName ActionID);

	//현재 타겟 목록 설정
	UFUNCTION(BlueprintCallable, Category = "Player|Input")
	void SetCurrentTargets(const TArray<ACombatPawn*>& NewTargets);

};