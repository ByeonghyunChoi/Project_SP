// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/BattleTurnComponent.h"
#include "Combat/CharacterStatsComponent.h"
#include "Core/BattleManager.h"
#include "EngineUtils.h"


// Sets default values for this component's properties
UBattleTurnComponent::UBattleTurnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	BattleManagerRef = nullptr;
	bIsMyTurn = false;
	ActionValue = 0.0f;
	ActionThreshold = 10000.0f;
	StatsComp = nullptr;
}


// Called when the game starts
void UBattleTurnComponent::BeginPlay()
{
	Super::BeginPlay();

	StatsComp = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();

	for (TActorIterator<ABattleManager> It(GetWorld()); It; ++It)
	{
		BattleManagerRef = *It;
		break;
	}
}

bool UBattleTurnComponent::GetIsMyTurn() const
{
	return bIsMyTurn;
}

float UBattleTurnComponent::GetActionValue() const
{
	return ActionValue;
}

bool UBattleTurnComponent::IsReadyForTurn() const
{
	return ActionValue >= ActionThreshold;
}

float UBattleTurnComponent::GetTimeLeftToAct() const
{
	if (!StatsComp || StatsComp->GetMovementSpeed() <= 0.f)
	{
		return 99999.f;
	}

	float Remaining = ActionThreshold - ActionValue;
	return FMath::Max(0.f, Remaining / StatsComp->GetMovementSpeed());
}

void UBattleTurnComponent::StartTurn()
{
	bIsMyTurn = true;
	ActionValue = 0.f;
}

void UBattleTurnComponent::EndTurn()
{
	bIsMyTurn = false;
}

void UBattleTurnComponent::AdvanceActionValue(float DeltaTime)
{
	if (bIsMyTurn || !StatsComp)
		return;

	ActionValue += StatsComp->GetMovementSpeed() * DeltaTime;
}
