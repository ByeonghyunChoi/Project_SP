// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPawn.h"
#include "BattleManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACombatPawn::ACombatPawn()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StatsComponent = CreateDefaultSubobject<UCharacterStatsComponent>(TEXT("StatsComponent"));
	BattleTurnComponent = CreateDefaultSubobject<UBattleTurnComponent>(TEXT("BattleTurnComponent"));
}

void ACombatPawn::BeginPlay()
{
	Super::BeginPlay();
}


FString ACombatPawn::GetCharacterName() const
{
	return CharacterName;
}

void ACombatPawn::SetCharacterName(const FString& NewName)
{
	CharacterName = NewName;
}

EFaction ACombatPawn::GetFaction() const
{
	return Faction;
}

void ACombatPawn::SetFaction(EFaction NewFaction)
{
	Faction = NewFaction;
}

UCharacterStatsComponent* ACombatPawn::GetStatsComponent() const
{
	return StatsComponent;
}

UBattleTurnComponent* ACombatPawn::GetBattleTurnComponent() const
{ 
	return BattleTurnComponent; 
}


