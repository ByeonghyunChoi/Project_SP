// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleModeComponent.h"

// Sets default values for this component's properties
UBattleModeComponent::UBattleModeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UBattleModeComponent::ReceiveTurn()
{
	//턴 받기 알림 로직
}

void UBattleModeComponent::EndPlayerTurn()
{
	//턴 종료 알림 로직
}


// Called when the game starts
void UBattleModeComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UBattleModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

