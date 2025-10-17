#pragma once

#include "CoreMinimal.h"
#include "Character/CombatPawn.h"
#include "PlayerCharacter.generated.h"

class UWeaponSystemComponent;
class UFieldActionComponent;
class UPlayerCombatControlComponent;
class UInventoryComponent;
class UOpartsComponent;


UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 필드 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UFieldActionComponent* FieldActionComp;
	// 무기 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWeaponSystemComponent> WeaponSystemComponent;
	// 전투 행동 제어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brain")
	TObjectPtr<UPlayerCombatControlComponent> CombatControlComponent;
	// 인벤토리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComponent;
	//오파츠 시스템 컴포넌트


public:
	virtual void OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets) override;

	// 모드 전환 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Modes")
	void OnEnterFieldMode();

	UFUNCTION(BlueprintImplementableEvent, Category = "Modes")
	void OnEnterBattleMode();
};