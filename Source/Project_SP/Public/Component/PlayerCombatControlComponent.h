// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ActionData.h"
#include "Combat/CombatTypes.h"
#include "PlayerCombatControlComponent.generated.h"

class ACombatPawn;
class UInputAction;
class APlayerCharacter;
class UActionComponent;
class UEnhancedInputComponent;
class UWeaponSystemComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionSelected, FName, SelectedActionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetsChanged, const TArray<ACombatPawn*>&, NewTargets);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UPlayerCombatControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerCombatControlComponent();

	void SetupPlayerInput(UEnhancedInputComponent* PlayerInputComponent);

	void OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActionSelected OnActionSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTargetsChanged OnTargetsChanged;

	UFUNCTION()
	void OnReceiveParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration);

	UFUNCTION()
	void OnReceiveParryWindowClosed(ACombatPawn* Attacker);

protected:
	virtual void BeginPlay() override;
	// 컴포넌트 참조
	UPROPERTY() 
	TObjectPtr<APlayerCharacter> OwningPlayerCharacter;
	UPROPERTY() 
	TObjectPtr<UActionComponent> ActionComponent;
	UPROPERTY() TObjectPtr<UWeaponSystemComponent> WeaponSystemComponent;

	//패링 관련 기능과 데이터
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Parry")
	bool bIsParryWindowOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Parry")
	EDamageType RequiredParryType;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Parry")
	TWeakObjectPtr<ACombatPawn> CurrentParryAttacker;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Parry")
	bool bPendingParryInterrupt = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Parry")
	FName PendingParrySkillID = NAME_None;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Parry")
	TWeakObjectPtr<ACombatPawn> PendingParryTarget;

	bool bIsParrySequenceActive = false;

	// 전투 제어 변수
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<ACombatPawn>> AllEnemyTargets;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<ACombatPawn>> CurrentTargets;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	FName SelectedActionID;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	int32 CurrentTargetIndex;

	// 입력 액션
	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_SelectBasicAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_SelectMainSkill;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_ChangeFirstWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_ChangeSecondWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_ChangeThirdWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_ConfirmAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_CycleTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> IA_SelectTargetMouse;

private:
	//입력 처리 함수
	void HandleSelectBasicAttack(const FInputActionValue& Value);
	void HandleSelectMainSkill(const FInputActionValue& Value);
	void HandleChangeWeapon(int32 WeaponIndex);
	void HandleConfirmAction(const FInputActionValue& Value);
	void HandleCycleTarget(const FInputActionValue& Value);
	void HandleSelectTargetMouse(const FInputActionValue& Value);

	//로직 처리 함수
	UFUNCTION(BlueprintCallable)
	void SelectAction(FName ActionID);

	void ConfirmAndExecuteAction();
	void BeginTargetSelection();
	void CycleTarget(float Direction);
	void SelectTargetByMouse();
	void SetCurrentTargets(const TArray<ACombatPawn*>& NewTargets);	

	void OnParrySuccess(ACombatPawn* ParriedAttacker);
	void OnParryFailure(ACombatPawn* ParriedAttacker, EParryResult Result);

};
