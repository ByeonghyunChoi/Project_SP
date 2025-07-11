// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"


UCharacterBase::UCharacterBase()
{
	bIsMyTurn = false;
	CharacterName = "Default Character";
	CharacterFaction = EFaction::Player;
	fActionValue = 0.0f;
}

void UCharacterBase::StartTurn()
{
	bIsMyTurn = true;
	fActionValue = 0.0f;
	//턴 행동
}

void UCharacterBase::EndTurn()
{
	bIsMyTurn = false;
}

bool UCharacterBase::GetIsMyTurn() const
{
	return bIsMyTurn;
}

const FCharacterStatsData& UCharacterBase::GetStats() const
{
	return Stats;
}

const FString& UCharacterBase::GetCharacterName() const
{
	return CharacterName;
}

float UCharacterBase::GetTimeLeftToAct() const
{
	const float TargetDistance = 10000.0f;
	float RemainingDistance = TargetDistance - fActionValue;

	// 속도가 0이거나 음수일 경우를 방지 (0으로 나누기 오류)
	if (Stats.fSpeed <= 0.0f)
	{
		return 99999.0f; // 속도 0이면 무한대 시간
	}

	// 남은 거리가 음수일 경우 (이미 목적지를 지났을 경우) 0으로 처리
	return FMath::Max(0.0f, RemainingDistance / Stats.fSpeed);
}

void UCharacterBase::AdvanceActionValue(float TimeIncrement)
{
	if (!bIsMyTurn) // 자신의 턴이 아닐 때만 거리 증가
	{
		fActionValue += Stats.fSpeed * TimeIncrement;
	}
}

bool UCharacterBase::IsReadyForTurn() const
{
	const float TargetDistance = 10000.0f;
	return fActionValue >= TargetDistance;
}

float UCharacterBase::GetActionValue() const
{
	return fActionValue;
}

EFaction UCharacterBase::GetFaction() const
{
	return CharacterFaction;
}

void UCharacterBase::SetFaction(EFaction InFaction)
{
	CharacterFaction = InFaction;
}

void UCharacterBase::SetStats(const EStat& ChangeStat, const float InAmount)
{
	switch (ChangeStat)
	{
	case EStat::CurrentHealth:
		Stats.fCurrentHealth = FMath::Clamp(InAmount, 0.0f, Stats.fMaxHealth);;
		break;
	case EStat::MaxHealth:
		Stats.fMaxHealth = InAmount;
		break;
	case EStat::AttackPower:
		Stats.fAttackPower = InAmount;
		break;
	case EStat::DefensePower:
		Stats.fDefensePower = InAmount;
		break;
	case EStat::Speed:
		Stats.fSpeed = InAmount;
		break;
	case EStat::CriticalChance:
		Stats.fCriticalChance = InAmount;
		break;
	case EStat::CriticalDamageMultiplier:
		Stats.fCriticalDamageMultiplier = InAmount;
		break;
	case EStat::Accuracy:
		Stats.fAccuracy = InAmount;
		break;
	case EStat::Evasion:
		Stats.fEvasion = InAmount;
		break;
	case EStat::StatusEffectResistance:
		Stats.fStatusEffectResistance = InAmount;
		break;
	case EStat::StatusEffectAccuracy:
		Stats.fStatusEffectAccuracy = InAmount;
		break;
	case EStat::DamageIncreaseMultiplier:
		Stats.fDamageIncreaseMultiplier = InAmount;
		break;
	case EStat::DamageReductionMultiplier:
		Stats.fDamageReductionMultiplier = InAmount;
		break;
	case EStat::ArmorPenetration:
		Stats.fArmorPenetration = InAmount;
		break;
	}
}


void UCharacterBase::CopyStats(FCharacterStatsData InStats)
{
	Stats = InStats;
}

void UCharacterBase::SetCharacterName(const FString& NewName)
{
	CharacterName = NewName;
}
