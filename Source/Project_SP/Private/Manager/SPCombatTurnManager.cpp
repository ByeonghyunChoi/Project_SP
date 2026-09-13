// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SPCombatTurnManager.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerController.h"

// 턴 시뮬레이션 구조체
struct FSimulatedActor
{
	AActor* Actor;
	float Speed;
	float Gauge;
};


ASPCombatTurnManager::ASPCombatTurnManager()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = false;
}

void ASPCombatTurnManager::InitializeParticipants(const TArray<AActor*>& InParticipants)
{
	Participants = InParticipants;
	TurnQueue.Empty();
}

FTurnResult ASPCombatTurnManager::CalculateNextTurn()
{
	// 대기열 확인
	while (InterruptQueue.Num() > 0)
	{
		AActor* InterruptActor = InterruptQueue[0];
		InterruptQueue.RemoveAt(0);

		if (IsValid(InterruptActor) && Participants.Contains(InterruptActor))
		{
			return { InterruptActor, 0.0f };
		}
	}

	while (TurnQueue.Num() > 0)
	{
		AActor* NextActor = TurnQueue[0];
		TurnQueue.RemoveAt(0);

		if (IsValid(NextActor)
			&& Participants.Contains(NextActor)
			&& GetActionGauge(NextActor) >= MaxActionGauge - KINDA_SMALL_NUMBER)
		{
			return { NextActor, 0.0f };
		}
	}

	// 다음 캐릭터가 행동 가능해질 때까지 필요한 시간 계산
	float MinTimeToAct = TNumericLimits<float>::Max();
	bool bFoundValidActor = false;

	for (AActor* Actor : Participants)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		const float Speed = GetSpeed(Actor);
		const float CurrentGauge = GetActionGauge(Actor);

		// 속도가 0 이하면 행동 불가
		if (Speed <= 0.0f)
		{
			continue;
		}

		float TimeNeeded = (MaxActionGauge - CurrentGauge) / Speed;
		TimeNeeded = FMath::Max(0.0f, TimeNeeded);

		if (TimeNeeded < MinTimeToAct)
		{
			MinTimeToAct = TimeNeeded;
			bFoundValidActor = true;
		}
	}

	// 행동할 수 있는 유닛이 아무도 없음 (전원 속도 0 등)
	if (!bFoundValidActor)
	{
		return {};
	}

	// 계산된 시간만큼 모든 참가자의 ActionGauge 진행
	for (AActor* Actor : Participants)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		const float Speed = GetSpeed(Actor);
		const float CurrentGauge = GetActionGauge(Actor);
		const float NewGauge =
			CurrentGauge + (Speed * MinTimeToAct);

		SetActionGauge(Actor, NewGauge);

		if (NewGauge >= MaxActionGauge - KINDA_SMALL_NUMBER)
		{
			TurnQueue.AddUnique(Actor);
		}
	}

	// 동시에 행동 가능해진 캐릭터들의 우선순위 결정
	if (TurnQueue.Num() > 1)
	{
		TurnQueue.Sort(
			[this](const AActor& A, const AActor& B)
			{
				const float SpeedA = GetSpeed(&A);
				const float SpeedB = GetSpeed(&B);

				//1. 속도가 더 빠른쪽이 먼저 행동
				if (!FMath::IsNearlyEqual(SpeedA, SpeedB))
				{
					return SpeedA > SpeedB;
				}

				const bool bPlayerA =
					A.IsA(ASPGASPlayerCharacter::StaticClass());

				const bool bPlayerB =
					B.IsA(ASPGASPlayerCharacter::StaticClass());

				//2. 속도가 같은 경우 플레이어가 먼저 행동
				if (bPlayerA != bPlayerB)
				{
					return bPlayerA;
				}

				//3. 몬스터끼리인 경우 행동 게이지가 더 높은 쪽이 먼저 행동
				return GetActionGauge(&A) > GetActionGauge(&B);
			});
	}

	// 최종 행동자 꺼내기
	while (TurnQueue.Num() > 0)
	{
		AActor* NextActor = TurnQueue[0];
		TurnQueue.RemoveAt(0);

		if (IsValid(NextActor) && Participants.Contains(NextActor))
		{
			return { NextActor, MinTimeToAct };
		}
	}

	return { nullptr, MinTimeToAct };
}

void ASPCombatTurnManager::RemoveParticipant(AActor* DeadActor)
{
	if (!IsValid(DeadActor)) return;

	if (Participants.Contains(DeadActor))
	{
		Participants.Remove(DeadActor);
	}
	if (TurnQueue.Contains(DeadActor))
	{
		TurnQueue.Remove(DeadActor);
	}
}

void ASPCombatTurnManager::AddParticipant(AActor* NewActor)
{
	if (!NewActor) return;

	// 🌟 현재 라운드가 진행 중(루프 내부)이라면 메인 배열을 건드리지 않고 대기열로 보낸다!
	if (bIsRoundIterating)
	{
		PendingParticipants.Add(NewActor);
		UE_LOG(LogTemp, Warning, TEXT("[TurnManager] 전투 진행 중 소환 발생! 임시 대기열에 예약됨: %s"), *NewActor->GetName());
	}
	else
	{
		// 전투 시작 전이거나 라운드 정비 타임일 때는 즉시 추가
		Participants.Add(NewActor);
	}
}

float ASPCombatTurnManager::GetSpeed(const AActor* Target) const
{
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// 버프/디버프가 적용된 최종 Speed 값 반환
			return FMath::Max(0.0f, ASC->GetNumericAttribute(USPGASAttributeSet::GetSpeedAttribute()));
		}
	}
	return 0.0f;
}

float ASPCombatTurnManager::GetActionGauge(const AActor* Target) const
{
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			return ASC->GetNumericAttribute(USPGASAttributeSet::GetActionGaugeAttribute());
		}
	}
	return 0.0f;
}

void ASPCombatTurnManager::ConsumeTurn(AActor* Target, ETurnConsumePolicy ConsumePolicy)
{
	if (!IsValid(Target))
	{
		return;
	}

	ClearActorFromQueue(Target);

	const float BeforeGauge = GetActionGauge(Target);

	if (ConsumePolicy == ETurnConsumePolicy::PreserveGauge)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[TurnManager] %s Gauge 보존: %.1f"),
			*Target->GetName(),
			BeforeGauge);

		return;
	}

	const float CurrentGauge = GetActionGauge(Target);

	const float OverflowGauge = FMath::Max(
		0.0f,
		CurrentGauge - MaxActionGauge);

	SetActionGauge(Target, OverflowGauge);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[TurnManager] %s Gauge 소비: %.1f -> %.1f"),
		*Target->GetName(),
		BeforeGauge,
		OverflowGauge);
}


void ASPCombatTurnManager::SetActionGauge(AActor* Target, float NewValue)
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// 게이지 설정 (Base 값 변경)
			float ClampedValue = FMath::Max(0.0f, NewValue);
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), ClampedValue);
		}
	}
}

TArray<AActor*> ASPCombatTurnManager::PredictTurnOrder(int32 PredictionCount, float AVToCycleEnd, int32& OutCycleEndIndex)
{
	OutCycleEndIndex = -1; // -1이면 화면 안에 라운드 종료 선이 없다는 뜻
	TArray<AActor*> PredictedOrder;
	float TotalSimTime = 0.0f; // 시뮬레이션에서 흐른 누적 가상 시간

	// 1. 이미 대기열에 있는 유닛 (시간 흐르지 않음)
	for (AActor* QueuedActor : TurnQueue)
	{
		if (PredictedOrder.Num() >= PredictionCount) return PredictedOrder;
		PredictedOrder.Add(QueuedActor);
	}

	// 2. 가상 게이지 리스트 세팅
	TArray<FSimulatedActor> SimList;
	for (AActor* Actor : Participants)
	{
		if (IsValid(Actor)) SimList.Add({ Actor, GetSpeed(Actor), GetActionGauge(Actor) });
	}

	// 3. 평행우주 시간 돌리기
	while (PredictedOrder.Num() < PredictionCount)
	{
		float MinTime = TNumericLimits<float>::Max();
		bool bValid = false;

		for (const FSimulatedActor& Sim : SimList)
		{
			if (Sim.Speed > 0.0f)
			{
				float TimeNeeded = (MaxActionGauge - Sim.Gauge) / Sim.Speed;
				if (TimeNeeded < 0.0f) TimeNeeded = 0.0f;
				if (TimeNeeded < MinTime)
				{
					MinTime = TimeNeeded;
					bValid = true;
				}
			}
		}

		if (!bValid) break;

		// 다음 턴이 오기 전에 데드라인을 넘는다면?!
		if (TotalSimTime + MinTime >= AVToCycleEnd - KINDA_SMALL_NUMBER)
		{
			OutCycleEndIndex = PredictedOrder.Num(); // 전광판이 들어갈 위치 기록!
			return PredictedOrder; // 💥 여기서 예측을 강제 종료하고 배열 반환!
		}

		// 시간을 흐르게 합니다
		TotalSimTime += MinTime;

		TArray<int32> WinnerIndices;
		for (int32 i = 0; i < SimList.Num(); ++i)
		{
			if (SimList[i].Speed > 0.0f)
			{
				SimList[i].Gauge += (SimList[i].Speed * MinTime);
				if (SimList[i].Gauge >= MaxActionGauge - 0.01f) WinnerIndices.Add(i);
			}
		}

		if (WinnerIndices.Num() > 1)
		{
			WinnerIndices.Sort([&SimList](const int32 A, const int32 B) {
				const FSimulatedActor& SimA = SimList[A];
				const FSimulatedActor& SimB = SimList[B];
				if (!FMath::IsNearlyEqual(SimA.Speed, SimB.Speed)) return SimA.Speed > SimB.Speed;
				bool bPlayerA = SimA.Actor->IsA(ASPGASPlayerCharacter::StaticClass());
				bool bPlayerB = SimB.Actor->IsA(ASPGASPlayerCharacter::StaticClass());
				if (bPlayerA != bPlayerB) return bPlayerA;
				return SimA.Gauge > SimB.Gauge;
				});
		}

		for (int32 WinnerIndex : WinnerIndices)
		{
			if (PredictedOrder.Num() >= PredictionCount) break;
			PredictedOrder.Add(SimList[WinnerIndex].Actor);
			SimList[WinnerIndex].Gauge -= MaxActionGauge;
		}
	}

	return PredictedOrder;
}

void ASPCombatTurnManager::RequestInterruptTurn(AActor* Interrupter)
{
	if (!IsValid(Interrupter) || !Participants.Contains(Interrupter))
	{
		return; // 유효하지 않으면 즉시 종료
	}

	InterruptQueue.Add(Interrupter);
	UE_LOG(LogTemp, Warning, TEXT("[TurnManager] %s 가 새치기(인터럽트) 턴을 예약했습니다!"), *Interrupter->GetName());

	OnTurnOrderChanged.Broadcast();
}

AActor* ASPCombatTurnManager::PopInterruptActor()
{
	if (InterruptQueue.Num() > 0)
	{
		AActor* VIP = InterruptQueue[0];
		InterruptQueue.RemoveAt(0);
		return VIP;
	}
	return nullptr;
}

void ASPCombatTurnManager::ClearActorFromQueue(AActor* Target)
{
	if (TurnQueue.Contains(Target))
	{
		TurnQueue.Remove(Target);
	}
}

void ASPCombatTurnManager::SetRoundIterating(bool bIsIterating)
{
	bIsRoundIterating = bIsIterating;

	// 만약 누군가의 행동이 끝나서 라운드가 멈췄다면(false), 이때 대기열을 병합합니다!
	if (!bIsRoundIterating)
	{
		MergePendingParticipants();
	}
}

void ASPCombatTurnManager::MergePendingParticipants()
{
	if (PendingParticipants.Num() > 0)
	{
		for (AActor* PendingMinion : PendingParticipants)
		{
			if (IsValid(PendingMinion))
			{
				Participants.Add(PendingMinion);

				// 🌟 중요: 갓 태어난 소환수는 이번 턴 사이클에 새치기하지 못하게 게이지를 0으로 맞춥니다!
				SetActionGauge(PendingMinion, 0.0f);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[TurnManager] %d 마리의 소환수가 정식 명단에 합류했습니다!"), PendingParticipants.Num());
		PendingParticipants.Empty();

		// 턴 UI 업데이트 방송
		OnTurnOrderChanged.Broadcast();
	}
}
