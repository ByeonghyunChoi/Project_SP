// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SPCombatTurnManager.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Character/SPGASPlayerCharacter.h"


ASPCombatTurnManager::ASPCombatTurnManager()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ASPCombatTurnManager::InitializeParticipants(const TArray<AActor*>& InParticipants)
{
	Participants = InParticipants;
	TurnQueue.Empty();
}

AActor* ASPCombatTurnManager::CalculateNextTurn()
{
	if (TurnQueue.Num() > 0)
	{
		AActor* NextActor = TurnQueue[0];
		TurnQueue.RemoveAt(0);

		//유효하면 반환 그렇지 않으면 다시 시도
		if (IsValid(NextActor) && Participants.Contains(NextActor))
		{
			return NextActor;
		}
		return CalculateNextTurn();
	}

	float MinTimeToAct = 9999.0f;
	bool bFoundValidActor = false;

	for (AActor* Actor : Participants)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		float Speed = GetSpeed(Actor);
		float CurrentGauge = GetActionGauge(Actor);

		if (Speed > 0.0f)
		{
			float TimeNeeded = (MaxActionGauge - CurrentGauge) / Speed;

			//예외 처리
			if (TimeNeeded < 0.f)
			{
				TimeNeeded = 0.f;
			}

			if (TimeNeeded < MinTimeToAct)
			{
				MinTimeToAct = TimeNeeded;
				bFoundValidActor = true;
			}
		}
	}

	if (!bFoundValidActor) return nullptr;

	for (AActor* Actor : Participants)
	{
		if (!Actor)
		{
			continue;
		}

		float Speed = GetSpeed(Actor);
		float CurrentGauge = GetActionGauge(Actor);

		float NewGauge = CurrentGauge + (Speed * MinTimeToAct);

		SetActionGauge(Actor, NewGauge);

		//오차 보정 float를 사용하기 때문
		if (NewGauge >= MaxActionGauge - 0.01f)
		{
			//중복 참여 방지
			TurnQueue.AddUnique(Actor);
		}
	}

	if (TurnQueue.Num() > 1)
	{
		TurnQueue.Sort([this](const AActor& A, const AActor& B) {

			// 1. 속도 가져오기
			float SpeedA = GetSpeed(const_cast<AActor*>(&A));
			float SpeedB = GetSpeed(const_cast<AActor*>(&B));

			// 속도가 더 높은 캐릭터가 우선
			// (부동소수점 오차를 무시하고 거의 같지 않다면 비교)
			if (!FMath::IsNearlyEqual(SpeedA, SpeedB))
			{
				return SpeedA > SpeedB;
			}

			// 속도가 같다면 플레이어 우선
			// (A가 플레이어인지, B가 플레이어인지 확인)
			bool bIsPlayerA = A.IsA(ASPGASPlayerCharacter::StaticClass());
			bool bIsPlayerB = B.IsA(ASPGASPlayerCharacter::StaticClass());

			// 둘 중 하나만 플레이어라면, 플레이어인 쪽이 우선(true)
			if (bIsPlayerA != bIsPlayerB)
			{
				return bIsPlayerA;
			}

			//둘 다 적인 경우 ID순으로 처리
			return GetActionGauge(const_cast<AActor*>(&A)) > GetActionGauge(const_cast<AActor*>(&B));
			});
	}

	return CalculateNextTurn();
}

void ASPCombatTurnManager::RemoveParticipant(AActor* DeadActor)
{
	if (!IsValid(DeadActor)) return;

	// 참가자 목록에서 제거
	if (Participants.Contains(DeadActor))
	{
		Participants.Remove(DeadActor);
	}

	// 대기열에 있다면 제거
	if (TurnQueue.Contains(DeadActor))
	{
		TurnQueue.Remove(DeadActor);
	}
}


float ASPCombatTurnManager::GetSpeed(AActor* Target) const
{
	if (!IsValid(Target)) return 0.0f;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			//최종 스피드 값을 가져옴
			float Speed = ASC->GetNumericAttribute(USPGASAttributeSet::GetSpeedAttribute());
			return FMath::Max(0.0f, Speed);
		}
	}

	// GAS 컴포넌트가 없는 액터라면 0 반환 (행동 불가)
	return 0.0f;
}

float ASPCombatTurnManager::GetActionGauge(AActor* Target) const
{
	if (!IsValid(Target)) return 0.0f;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// 현재 행동 게이지 값 가져오기
			return ASC->GetNumericAttribute(USPGASAttributeSet::GetActionGaugeAttribute());
		}
	}
	return 0.0f;
}

void ASPCombatTurnManager::SetActionGauge(AActor* Target, float NewValue)
{
	if (!IsValid(Target)) return;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			float ClampedValue = FMath::Max(0.0f, NewValue);

			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), ClampedValue);
		}
	}
}
