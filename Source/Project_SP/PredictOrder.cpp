// Fill out your copyright notice in the Description page of Project Settings.


#include "PredictOrder.h"

// Called when the game starts
void UPredictOrder::BeginPlay()
{
	Super::BeginPlay();
	
	// 컴포넌트가 붙어있는 액터가 ABattleManager인지 확인하고 참조 저장
	BattleManagerRef = Cast<ABattleManager>(GetOwner());
	if (BattleManagerRef)
	{
		// ABattleManager의 OnTurnOrderChanged 델리게이트에 바인딩
		// ABattleManager가 턴 순서 변경을 알리면 RequestTurnOrderUpdate가 호출됩니다.
		BattleManagerRef->OnTurnOrderChanged.AddDynamic(this, &UPredictOrder::RequestTurnOrderUpdate);
		UE_LOG(LogTemp, Log, TEXT("UPredictOrder: PredictOrder연결됨"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPredictOrder: 이 컴포넌트는 ABattleManager에 붙어있어야 합니다."));
	}
}

TArray<ACombatPawn*> UPredictOrder::GetPredictedTurnOrder() const
{
	TArray<ACombatPawn*> PredictedOrderList; // 최종 반환할 예측 턴 목록

	// BattleManagerRef가 유효하지 않거나 전투 참여자가 없으면 빈 목록 반환
	if (!BattleManagerRef || BattleManagerRef->AllCombatants.Num() == 0) return PredictedOrderList;

	// 1. 현재 전투 참여자들의 상태를 복사하여 가상 시뮬레이션에 사용
	TArray<FSimulatedCombatantData> SimulatedCombatants;
	for (ACombatPawn* Combatant : BattleManagerRef->AllCombatants)
	{
		// 살아있는 캐릭터만 시뮬레이션에 포함
		if (Combatant && Combatant->GetCombatData() && Combatant->GetCombatData()->GetStats().fCurrentHealth > 0)
		{
			SimulatedCombatants.Emplace(Combatant); // FSimulatedCombatantData 생성자를 호출하여 데이터 복사
		}
	}

	// 2. 가상 시간 진행 및 턴 시뮬레이션
	const float VirtualGlobalTime = 0.0f; // 시뮬레이션 시작 시간은 0으로 가정 (상대적 시간)
	const int32 MaxTurnsToPredict = 4; // 예측할 최대 턴 수 

	// 시뮬레이션 루프
	for (int32 TurnCount = 0; TurnCount < MaxTurnsToPredict; ++TurnCount)
	{
		// 2.1. 현재 시뮬레이션 상태에서 누가 가장 먼저 턴을 잡을 준비가 되었는지 계산
		float MinTimeToNextTurn = 99999.0f; // 다음 턴까지 필요한 최소 시간

		// 살아있는 캐릭터 중 가장 TimeLeftToAct가 작은 값을 찾음
		bool bAnyCombatantAlive = false;
		for (const FSimulatedCombatantData& SimData : SimulatedCombatants)
		{
			if (SimData.bIsAlive && SimData.OriginalSpeed > 0) // 살아있고 속도가 있는 캐릭터만
			{
				bAnyCombatantAlive = true;
				float TimeToReach10000 = SimData.GetSimulatedTimeLeftToAct();
				if (TimeToReach10000 < MinTimeToNextTurn)
				{
					MinTimeToNextTurn = TimeToReach10000;
				}
			}
		}

		// 더 이상 살아있는 캐릭터가 없거나, 턴을 잡을 수 없는 경우 시뮬레이션 중단
		if (!bAnyCombatantAlive || MinTimeToNextTurn >= 99999.0f)
		{
			break;
		}

		// 2.2. MinTimeToNextTurn만큼 모든 캐릭터의 fActionValue를 가상으로 업데이트
		// MinTimeToNextTurn이 0보다 작거나 같을 수 있으므로 Max(0.0f, ...)로 방어
		float TimeToAdvance = FMath::Max(0.0f, MinTimeToNextTurn);

		for (FSimulatedCombatantData& SimData : SimulatedCombatants) // 참조로 받아야 SimData.SimulatedActionValue 변경 가능
		{
			if (SimData.bIsAlive)
			{
				SimData.SimulatedActionValue += SimData.OriginalSpeed * TimeToAdvance;
			}
		}

		// 2.3. 가상 시간 진행 후, 턴을 잡을 준비가 된 캐릭터들 모으기
		TArray<FSimulatedCombatantData*> ReadyForVirtualTurn; // 포인터 배열 (정렬에 용이)
		for (FSimulatedCombatantData& SimData : SimulatedCombatants)
		{
			if (SimData.bIsAlive && SimData.SimulatedActionValue >= 10000.0f)
			{
				ReadyForVirtualTurn.Add(&SimData);
			}
		}

		// 2.4. 턴을 잡을 캐릭터가 있다면 목록에 추가하고 fActionValue 리셋
		if (ReadyForVirtualTurn.Num() > 0)
		{
			// 정의된 비교자(FCompareSimulatedCombatantData)를 사용하여 우선순위 정렬
			ReadyForVirtualTurn.Sort(FCompareSimulatedCombatantData());

			FSimulatedCombatantData* NextSimulatedActor = ReadyForVirtualTurn[0];

			// 최종 예측 목록에 원본 ACombatPawn 추가
			if (NextSimulatedActor->OriginalCombatant.IsValid())
			{
				PredictedOrderList.Add(NextSimulatedActor->OriginalCombatant.Get());
			}
			else // 원본 액터가 유효하지 않으면 시뮬레이션에서 제외
			{
				NextSimulatedActor->bIsAlive = false;
				continue; // 다음 턴 예측으로 넘어감
			}

			// 턴을 마친 캐릭터의 fActionValue를 0으로 리셋 (가상 시뮬레이션에서만)
			NextSimulatedActor->SimulatedActionValue = 0.0f;
		}
		else
		{
			// 더 이상 예측할 캐릭터가 없거나 모든 캐릭터가 죽었을 경우
			break;
		}
	}

	return PredictedOrderList;
}

void UPredictOrder::RequestTurnOrderUpdate()
{
	if (OnTurnOrderChanged.IsBound())
	{
		OnTurnOrderChanged.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("UPredictOrder: 턴 순서 업데이트 요청 브로드캐스트."));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UPredictOrder: 바인딩 실패"));
	}
	
}

bool UPredictOrder::IsMyOnTurnOrderChangedBound() const
{
	return OnTurnOrderChanged.IsBound();
}


