#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PredictOrder.generated.h"

class ABattleManager;
class ACombatPawn;

/**
 * @class UPredictOrder
 * @brief BattleManager의 '턴 스택'과 캐릭터들의 '행동 게이지'를 종합하여
 * 미래의 턴 순서를 예측하는 컴포넌트입니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UPredictOrder : public UActorComponent
{
    GENERATED_BODY()

public:
    UPredictOrder();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TObjectPtr<ABattleManager> BattleManagerRef;

public:
    /**
     * @brief 현재 턴 스택과 미래 시뮬레이션을 조합하여 정확한 턴 순서를 예측합니다.
     * @param MaxPredictionCount 예측할 최대 턴 수
     * @return 예측된 ACombatPawn의 배열
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PredictOrder")
    TArray<ACombatPawn*> GetPredictedTurnOrder(int32 MaxPredictionCount = 5) const;
};