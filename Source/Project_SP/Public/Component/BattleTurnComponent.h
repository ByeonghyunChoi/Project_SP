#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BattleTurnComponent.generated.h"

class UAttributesComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UBattleTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBattleTurnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 현재 자신의 턴인지 여부
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Turn")
	bool bIsMyTurn = false;

	// 현재 행동 게이지 값
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Turn")
	float ActionValue = 0.0f;

	// 턴을 얻기 위해 도달해야 하는 행동 게이지 목표치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turn")
	float ActionThreshold = 10000.0f;

	// 속도 값을 가져오기 위한 AttributesComponent 참조
	UPROPERTY()
	TObjectPtr<UAttributesComponent> AttributesComp;

public:
	/** @brief 턴을 시작할 때 BattleManager에 의해 호출됩니다. 행동 게이지를 리셋합니다. */
	void StartTurn();

	/** @brief 턴을 종료할 때 BattleManager에 의해 호출됩니다. */
	void EndTurn();

	/** @brief 매 프레임 또는 일정 시간마다 호출되어 행동 게이지를 증가시킵니다. */
	void AdvanceActionValue(float DeltaTime);

	float GetActionValue();

	float GetActionThreshold();

	/** @brief 행동 게이지가 목표치에 도달했는지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "Turn")
	bool IsReadyForTurn() const;

};
