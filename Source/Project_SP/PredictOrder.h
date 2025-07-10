// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatPawn.h" 
#include "BattleManager.h"
#include "PredictOrder.generated.h"


// 시뮬레이션에 사용할 데이터 구조체 정의 (UPredictOrder 클래스 외부에 정의해도 됨)
	// UCharacterBase의 데이터를 복사하여 시뮬레이션에 사용합니다.
USTRUCT(BlueprintType)
struct FSimulatedCombatantData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<ACombatPawn> OriginalCombatant; // 원본 CombatPawn 참조 (UI 표시용)

	UPROPERTY(BlueprintReadWrite)
	float SimulatedActionValue; // 시뮬레이션 중인 fActionValue

	UPROPERTY(BlueprintReadWrite)
	float OriginalSpeed; // 원본 속도 (fSpeed)

	UPROPERTY(BlueprintReadWrite)
	EFaction OriginalFaction; // 원본 진영

	UPROPERTY(BlueprintReadWrite)
	bool bIsAlive; // 생존 여부 시뮬레이션

	// 기본 생성자
	FSimulatedCombatantData()
		: SimulatedActionValue(0.0f)
		, OriginalSpeed(0.0f)
		, OriginalFaction(EFaction::Player)
		, bIsAlive(false)
	{
	}

	// UCharacterBase 데이터를 복사하는 생성자
	FSimulatedCombatantData(ACombatPawn* InOriginalCombatant)
		: OriginalCombatant(InOriginalCombatant)
		, SimulatedActionValue(0.0f) 
		, OriginalSpeed(0.0f)        
		, OriginalFaction(EFaction::Player) 
		, bIsAlive(false)             
	{
		if (InOriginalCombatant && InOriginalCombatant->GetCombatData())
		{
			UCharacterBase* CombatData = InOriginalCombatant->GetCombatData();
			SimulatedActionValue = CombatData->GetActionValue();
			OriginalSpeed = CombatData->GetStats().fSpeed;
			OriginalFaction = CombatData->GetFaction();
			bIsAlive = CombatData->GetStats().fCurrentHealth > 0;
		}
	}

	// 다음 턴까지 남은 시간을 계산 (시뮬레이션용)
	FORCEINLINE float GetSimulatedTimeLeftToAct() const
	{
		if (OriginalSpeed <= 0.0f) return 99999.0f; // 속도 0이면 무한대 시간
		const float TargetDistance = 10000.0f;
		float RemainingDistance = TargetDistance - SimulatedActionValue;
		return FMath::Max(0.0f, RemainingDistance / OriginalSpeed);
	}
};


UCLASS( ClassGroup=(Custom), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UPredictOrder : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPredictOrder(): BattleManagerRef(nullptr)
	{
		// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
		// off to improve performance if you don't need them.
		PrimaryComponentTick.bCanEverTick = false;
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// 턴 순서 변경 시 UI에 알릴 델리게이트 객체 (PredictOrder가 Broadcast하고 UI가 구독)
	UPROPERTY(BlueprintAssignable, Category = "PredictOrder|Events")
	FOnTurnOrderChanged OnTurnOrderChanged;

	// ABattleManager로부터 전투 참여자 목록을 가져올 참조
	// 이 컴포넌트가 ABattleManager에 붙어있을 것이므로 GetOwner()로 가져올 수 있습니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PredictOrder", meta = (AllowPrivateAccess = "true"))
	class ABattleManager* BattleManagerRef; // <-- ABattleManager 참조

	// UI에 표시할 예상 턴 순서 목록을 반환 (가상 시뮬레이션 기반)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PredictOrder")
	TArray<ACombatPawn*> GetPredictedTurnOrder() const;

	// ABattleManager의 OnTurnOrderChanged 델리게이트에 바인딩하여 UI 업데이트를 트리거할 함수
	UFUNCTION(BlueprintCallable, Category = "PredictOrder")
	void RequestTurnOrderUpdate();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PredictOrder|Debug")
	bool IsMyOnTurnOrderChangedBound() const;
private:
	struct FCompareSimulatedCombatantData
	{
		FORCEINLINE bool operator()(const FSimulatedCombatantData& A, const FSimulatedCombatantData& B) const
		{
			// 1. 유효하지 않은 데이터 처리 (죽었거나 데이터 없는 경우 뒤로 보냄)
			// A와 B 모두 bIsAlive가 false인 경우, OriginalCombatant가 유효하지 않을 수 있으므로
			// Get()을 통해 얻은 포인터 값으로 안정 정렬 (nullptr도 비교 가능)
			if (!A.bIsAlive && !B.bIsAlive) return A.OriginalCombatant.Get() < B.OriginalCombatant.Get(); // <-- 수정
			// A만 bIsAlive가 false인 경우 (A를 뒤로 보냄)
			if (!A.bIsAlive) return true;
			// B만 bIsAlive가 false인 경우 (B를 뒤로 보냄)
			if (!B.bIsAlive) return false;

			// 이 시점에서는 A와 B 모두 bIsAlive가 true이므로 OriginalCombatant.Get()은 유효하다고 가정.
			// 하지만 방어적으로 Get() 결과가 nullptr인지 확인하는 것이 좋습니다.
			ACombatPawn* OriginalA = A.OriginalCombatant.Get();
			ACombatPawn* OriginalB = B.OriginalCombatant.Get();

			// 만약 OriginalCombatant가 nullptr이라면 (예외적인 상황), Get() 결과 포인터로 대체 정렬
			if (!OriginalA || !OriginalB) return OriginalA < OriginalB; // <-- 수정


			// GetSimulatedTimeLeftToAct()가 낮은 것이 우선 (오름차순)
			float TimeLeftA = A.GetSimulatedTimeLeftToAct();
			float TimeLeftB = B.GetSimulatedTimeLeftToAct();

			if (TimeLeftA != TimeLeftB)
			{
				return TimeLeftA < TimeLeftB;
			}
			else // TimeLeft가 같다면 다음 규칙 적용
			{
				// 2. 속도 비교 (속도 높은 순)
				if (A.OriginalSpeed != B.OriginalSpeed)
				{
					return A.OriginalSpeed > B.OriginalSpeed;
				}
				else // 속도도 같다면 진영 규칙 적용
				{
					// 규칙 5-1: 아군 진영이 우선
					if (A.OriginalFaction == EFaction::Player && B.OriginalFaction != EFaction::Player) return true;
					if (B.OriginalFaction == EFaction::Player && A.OriginalFaction != EFaction::Player) return false;

					// 그 외의 경우 (모든 조건이 같다면 고유 ID로 안정 정렬)
					return OriginalA->GetUniqueID() < OriginalB->GetUniqueID(); // <-- Get() 호출 후 GetUniqueID()
				}
			}
		}
	};

};
