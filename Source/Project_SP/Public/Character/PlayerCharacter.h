#pragma once

#include "CoreMinimal.h"
#include "Character/CombatPawn.h"
#include "Data/ActionData.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UWeaponSystemComponent;
class UFieldModeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionSelectedForTargeting, const FActionData&, ActionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, ACombatPawn*, NewTarget);

UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(BlueprintAssignable)
	FOnActionSelectedForTargeting OnActionSelectedForTargeting;

	UPROPERTY(BlueprintAssignable)
	FOnTargetChanged OnTargetChanged;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 필드 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UFieldModeComponent* FieldModeComp;
	// 무기 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWeaponSystemComponent> WeaponSystemComponent;

	// --- 이벤트 핸들러 ---
	// ACombatPawn의 GameEventComponent::OnTurnStarted 이벤트 구독
	UFUNCTION()
	void HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState);

	//상태 변수
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat")
	TArray<TObjectPtr<ACombatPawn>> CurrentTargets;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	FName SelectedActionID;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<ACombatPawn>> AllEnemyTargets;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	int32 CurrentTargetIndex;

	// 사용할 InputAction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_SelectBasicAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_SelectMainSkill;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_Weapon1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_Weapon2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_Weapon3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_ConfirmAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_SelectTargetMouse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<class UInputAction> IA_CycleTarget;

	//입력 처리 함수
	void HandleSelectBasicAttack(const FInputActionValue& Value);
	void HandleSelectMainSkill(const FInputActionValue& Value);
	void HandleWeapon1(const FInputActionValue& Value);
	void HandleWeapon2(const FInputActionValue& Value);
	void HandleWeapon3(const FInputActionValue& Value);
	void HandleConfirmAction(const FInputActionValue& Value);
	void HandleCycleTarget(const FInputActionValue& Value);
	void HandleSelectTargetMouse(const FInputActionValue& Value);

	//로직 처리 함수
	void SelectAction(FName ActionID);
	void ConfirmAndExecuteAction();
	void BeginTargetSelection();
	void CycleTarget(float Direction);
	void SelectTargetWithMouse();
	void HandleWeaponInput(int32 WeaponIndex);
	void AttemptSwitchParry(EDamageType ParryType);

public:
	virtual void OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets) override;

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