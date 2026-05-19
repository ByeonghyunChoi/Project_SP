// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SPCombatTurnManager.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Character/SPGASPlayerCharacter.h"
#include "Character/SPGASPlayerController.h"
#include "Game/ASPCombatGameMode.h"

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

AActor* ASPCombatTurnManager::CalculateNextTurn()
{
	// 대기열 확인
	if (InterruptQueue.Num() > 0)
	{
		AActor* VIPActor = InterruptQueue[0];
		InterruptQueue.RemoveAt(0);

		if (IsValid(VIPActor) && Participants.Contains(VIPActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("[TurnManager] VIP 턴 발동! %s 가 게이지를 무시하고 턴을 잡습니다!"), *VIPActor->GetName());
			return VIPActor;
		}
		return CalculateNextTurn(); // 죽었거나 유효하지 않으면 다음 타자 검색
	}

	if (TurnQueue.Num() > 0)
	{
		AActor* NextActor = TurnQueue[0];
		TurnQueue.RemoveAt(0);

		// 유효성 검사 (죽었거나 사라졌으면 재귀 호출로 다음 타자 찾기)
		if (IsValid(NextActor) && Participants.Contains(NextActor) && GetActionGauge(NextActor) >= MaxActionGauge - 0.1f)
		{
			return NextActor;
		}
		return CalculateNextTurn();
	}

	// 시뮬레이션: 가장 빨리 행동할 수 있는 시간(MinTimeToAct) 계산
	float MinTimeToAct = 99999.0f;
	bool bFoundValidActor = false;

	for (AActor* Actor : Participants)
	{
		if (!IsValid(Actor)) continue;

		float Speed = GetSpeed(Actor);
		float CurrentGauge = GetActionGauge(Actor);

		// 속도가 0 이하면 행동 불가
		if (Speed > 0.0f)
		{
			// 남은 거리 / 속도 = 걸리는 시간
			float TimeNeeded = (MaxActionGauge - CurrentGauge) / Speed;

			// 이미 100을 넘었으면 시간은 0
			if (TimeNeeded < 0.f) TimeNeeded = 0.f;

			if (TimeNeeded < MinTimeToAct)
			{
				MinTimeToAct = TimeNeeded;
				bFoundValidActor = true;
			}
		}
	}

	// 행동할 수 있는 유닛이 아무도 없음 (전원 속도 0 등)
	if (!bFoundValidActor) return nullptr;

	// 게임 모드에 행동 수치 경과 보고
	if (MinTimeToAct > 0.0f)
	{
		if (AASPCombatGameMode* CombatGM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
		{
			CombatGM->AdvanceBattleTime(MinTimeToAct);
		}
	}

	// 3. 시간 흐르기: 모든 유닛의 게이지 전진
	for (AActor* Actor : Participants)
	{
		if (!IsValid(Actor)) continue;

		float Speed = GetSpeed(Actor);
		float CurrentGauge = GetActionGauge(Actor);

		// 이동 거리 = 시간 * 속도
		float NewGauge = CurrentGauge + (Speed * MinTimeToAct);

		// 게이지 업데이트 (GAS Attribute 변경)
		SetActionGauge(Actor, NewGauge);

		// 게이지가 꽉 찼다면 대기열(Queue)에 추가
		// (부동소수점 오차 고려하여 0.01f 여유)
		if (NewGauge >= MaxActionGauge - 0.01f)
		{
			TurnQueue.AddUnique(Actor);
		}
	}

	// 4. 우선순위 정렬 (동시에 100 도달 시 누가 먼저인가?)
	if (TurnQueue.Num() > 1)
	{
		TurnQueue.Sort([this](const AActor& A, const AActor& B) {

			// 기준 1: 속도가 빠른 순서
			float SpeedA = GetSpeed(&A);
			float SpeedB = GetSpeed(&B);
			if (!FMath::IsNearlyEqual(SpeedA, SpeedB))
			{
				return SpeedA > SpeedB;
			}

			// 기준 2: 속도가 같다면 플레이어 우선 (유저 친화적)
			bool bPlayerA = A.IsA(ASPGASPlayerCharacter::StaticClass());
			bool bPlayerB = B.IsA(ASPGASPlayerCharacter::StaticClass());
			if (bPlayerA != bPlayerB)
			{
				return bPlayerA; // true(Player)가 앞으로 옴
			}

			// 기준 3: 그래도 같다면 현재 게이지가 더 높은 순 (오버플로우)
			return GetActionGauge(const_cast<AActor*>(&A)) > GetActionGauge(const_cast<AActor*>(&B));
			});
	}

	// 5. 대기열이 채워졌으니 다시 호출하여 1번 과정 수행
	return CalculateNextTurn();
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

TArray<AActor*> ASPCombatTurnManager::PredictTurnOrder(int32 PredictionCount, int32& OutCycleEndIndex)
{
	OutCycleEndIndex = -1; // -1이면 화면 안에 라운드 종료 선이 없다는 뜻
	TArray<AActor*> PredictedOrder;

	float TotalSimTime = 0.0f; // 시뮬레이션에서 흐른 누적 가상 시간
	float AVToCycleEnd = 999999.0f;

	if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		AVToCycleEnd = GM->GetAVToCycleEnd(); // 데드라인 가져오기
	}

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
		float MinTime = 99999.0f;
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

		// 💥 [핵심 단두대 로직] 다음 턴이 오기 전에 데드라인을 넘는다면?!
		// (부동소수점 오차 방지를 위해 0.01f 여유를 둡니다)
		if (TotalSimTime + MinTime >= AVToCycleEnd - 0.01f)
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
