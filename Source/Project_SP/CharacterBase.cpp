// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "BattleManager.h"


UCharacterBase::UCharacterBase()
{
	fActionGauge = 0.0f;
	bIsMyTurn = false;
	iTurnOrderIndex = 0;
}

void UCharacterBase::UpdateActionGauge(float Amount)
{
	if (!bIsMyTurn) 
	{
		fActionGauge += Amount;
		fActionGauge = FMath::Min(fActionGauge, 100.0f);
	}
}

void UCharacterBase::TakeDamage(float DamageAmount)
{
	Stats.fCurrentHealth -= DamageAmount;
	Stats.fCurrentHealth = FMath::Max(Stats.fCurrentHealth, 0.0f);

	if (Stats.fCurrentHealth <= 0.0f)
	{
		//사망
	}
}

void UCharacterBase::StartTurn()
{
	bIsMyTurn = true;
	fActionGauge = 0.0f;
	//턴 행동
}

void UCharacterBase::EndTurn()
{
	bIsMyTurn = false;
}


bool UCharacterBase::IsReadyForTurn()
{
	return fActionGauge >= 100.0f;
}

void UCharacterBase::DecideAction()
{
	//AI 로직 작동 BehaviorTree로 구현할 예정
}

EFaction UCharacterBase::GetFaction() const
{
	return CharacterFaction;
}

void UCharacterBase::SetFaction(EFaction inFaction)
{
	CharacterFaction = inFaction;
}

void UCharacterBase::SetStats(const FCharacterStatsData& NewStats)
{
	Stats = NewStats;
}

float UCharacterBase::GetActionGauge() const
{
	return fActionGauge;
}

bool UCharacterBase::GetIsMyTurn() const
{
	return bIsMyTurn;
}

int32 UCharacterBase::GetTurnOrderIndex() const
{
	return iTurnOrderIndex;
}

const FCharacterStatsData& UCharacterBase::GetStats() const
{
	return Stats;
}
