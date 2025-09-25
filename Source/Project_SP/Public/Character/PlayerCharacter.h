#pragma once

#include "CoreMinimal.h"
#include "Component/FieldModeComponent.h"
#include "Character/CombatPawn.h"
#include "Data/ActionData.h"
#include "InputActionValue.h"
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

	// 사용할 InputAction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SelectBasicAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SelectMainSkillAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> Weapon1Action;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> Weapon2Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> Weapon3Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> ConfirmActionAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SelectTargetAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> CycleTargetAction;

	//입력 처리 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SelectBasicAttack(const FInputActionValue& Value);
	void SelectMainSkill(const FInputActionValue& Value);
	void Weapon1(const FInputActionValue& Value);
	void Weapon2(const FInputActionValue& Value);
	void Weapon3(const FInputActionValue& Value);
	void ConfirmAction(const FInputActionValue& Value);
	void CycleTarget(const FInputActionValue& Value);
	void SelectTargetMouse(const FInputActionValue& Value);

public:
	virtual void OnTurnBegin() override;

	// 모드 전환 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Modes")
	void OnEnterFieldMode();

	UFUNCTION(BlueprintImplementableEvent, Category = "Modes")
	void OnEnterBattleMode();

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