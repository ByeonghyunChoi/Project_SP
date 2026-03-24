// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Tag/SPGameplayTags.h"
#include "SPGASAIController.generated.h"

struct FGameplayEventData;
/**
 * 
 */
UCLASS()
class PROJECT_SP_API ASPGASAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASPGASAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	void OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 턴 시작 이벤트 감지 (내 차례가 옴)
	void OnTurnStartEvent(const FGameplayEventData* Payload);

	// 턴 종료 지연 처리 함수
	void FinishTurnDelayed();

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBehaviorTree> FieldBT;

private:
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> CachedASC;
};
