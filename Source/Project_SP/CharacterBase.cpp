// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"


UCharacterBase::UCharacterBase()
{
	Stats = CreateDefaultSubobject<UCharacterStats>(TEXT("Stats"));
	bIsMyTurn = false;
	CharacterName = "Default Character";
	CharacterFaction = EFaction::None;
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

UCharacterStats* UCharacterBase::GetStats() const
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
	if (Stats->GetMovementSpeed() <= 0.0f)
	{
		return 99999.0f; // 속도 0이면 무한대 시간
	}

	// 남은 거리가 음수일 경우 (이미 목적지를 지났을 경우) 0으로 처리
	return FMath::Max(0.0f, RemainingDistance / Stats->GetMovementSpeed());
}

void UCharacterBase::AdvanceActionValue(float TimeIncrement)
{
	if (!bIsMyTurn) // 자신의 턴이 아닐 때만 거리 증가
	{
		fActionValue += Stats->GetMovementSpeed() * TimeIncrement;
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


void UCharacterBase::SetFaction(const EFaction& InFaction)
{
	CharacterFaction = InFaction;
}

void UCharacterBase::SetStats(UCharacterStats* InStats)
{
	if (InStats) // <-- InStats 유효성 검사 추가
	{
		Stats = InStats;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UCharacterBase: SetStats에 유효하지 않은 UCharacterStats*가 전달되었습니다."));
	}
}

void UCharacterBase::SetCharacterName(const FString& NewName)
{
	CharacterName = NewName;
}
