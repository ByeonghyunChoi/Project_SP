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
	void HandleMyPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState);

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

	virtual void OnTurnBegin() override;

};