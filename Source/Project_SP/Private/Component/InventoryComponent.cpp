// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Component/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// [테스트용] 개발 편의를 위해 초기 자원 지급 (나중에 삭제)
	Wallet.Sand = 1000;
	Wallet.IncompleteEnergy = 10;

	// 초기 상태 UI 갱신
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
}

void UInventoryComponent::AddSand(int32 Amount)
{
	if (Amount <= 0) return;
	Wallet.Sand += Amount;

	UE_LOG(LogTemp, Log, TEXT("모래 획득: +%d (현재: %d)"), Amount, Wallet.Sand);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
}

bool UInventoryComponent::ConsumeSand(int32 Amount)
{
	if (Amount <= 0) return false;
	if (Wallet.Sand < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("모래 부족! 필요: %d, 보유: %d"), Amount, Wallet.Sand);
		return false;
	}

	Wallet.Sand -= Amount;
	UE_LOG(LogTemp, Log, TEXT("모래 소모: -%d (남은 양: %d)"), Amount, Wallet.Sand);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
	return true;
}

void UInventoryComponent::AddIncompleteEnergy(int32 Amount)
{
	if (Amount <= 0) return;
	Wallet.IncompleteEnergy += Amount;

	UE_LOG(LogTemp, Log, TEXT("기운 획득: +%d (현재: %d)"), Amount, Wallet.IncompleteEnergy);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
}

bool UInventoryComponent::ConsumeIncompleteEnergy(int32 Amount)
{
	if (Amount <= 0) return false;
	if (Wallet.IncompleteEnergy < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("기운 부족! 필요: %d, 보유: %d"), Amount, Wallet.IncompleteEnergy);
		return false;
	}

	Wallet.IncompleteEnergy -= Amount;
	UE_LOG(LogTemp, Log, TEXT("기운 소모: -%d (남은 양: %d)"), Amount, Wallet.IncompleteEnergy);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
	return true;
}

void UInventoryComponent::AddMoney(int32 Amount)
{
	if (Amount <= 0) return;
	Wallet.Money += Amount;

	UE_LOG(LogTemp, Log, TEXT("골드 획득: +%d (현재: %d)"), Amount, Wallet.Money);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
}

bool UInventoryComponent::ConsumeMoney(int32 Amount)
{
	if (Amount <= 0) return false;
	if (Wallet.Money < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("골드 부족! 필요: %d, 보유: %d"), Amount, Wallet.Money);
		return false;
	}

	Wallet.Money -= Amount;
	UE_LOG(LogTemp, Log, TEXT("골드 소모: -%d (남은 양: %d)"), Amount, Wallet.Money);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Wallet);
	return true;
}
