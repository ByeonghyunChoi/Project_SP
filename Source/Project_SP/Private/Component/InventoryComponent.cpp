// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 테스트용 초기값
	PlayerMaterials.Sand = 0;
	PlayerMaterials.IncompleteEnergy = 0; // 테스트용 초기값
}

// 현재 모래 가져오기
int32 UInventoryComponent::GetCurrentSand() const
{
	return PlayerMaterials.Sand;
}

// 모래 개수 설정
void UInventoryComponent::SetCurrentSand(int32 NewAmount)
{
	// 최소값을 0으로 설정하여 음수 재화를 방지합니다.
	PlayerMaterials.Sand = FMath::Max(0, NewAmount);

}

// 현재 불완전한 기운 가져오기
int32 UInventoryComponent::GetCurrentIncompleteEnergy() const
{
	return PlayerMaterials.IncompleteEnergy;
}

// 불완전한 기운 설정
void UInventoryComponent::SetCurrentIncompleteEnergy(int32 NewAmount)
{
	PlayerMaterials.IncompleteEnergy = FMath::Max(0, NewAmount);
}

int32 UInventoryComponent::GetCurrentMoney() const
{
	return PlayerMaterials.Money;
}

void UInventoryComponent::SetCurrentMoney(int32 NewAmount)
{
	PlayerMaterials.Money = FMath::Max(0, NewAmount);
}

void UInventoryComponent::GainSand(int32 Amount)
{
	int32 ActrualReward = FMath::Max(0, Amount);
	int32 NewTotalSand = GetCurrentSand() + ActrualReward;
	SetCurrentSand(NewTotalSand);
}

void UInventoryComponent::GainIncompleteEnergy(int32 Amount)
{
	int32 ActrualReward = FMath::Max(0, Amount);
	int32 NewTotalIncompleteEnergy = GetCurrentIncompleteEnergy() + ActrualReward;
	SetCurrentIncompleteEnergy(NewTotalIncompleteEnergy);
}

void UInventoryComponent::GainMoney(int32 Amount)
{
	int32 ActrualReward = FMath::Max(0, Amount);
	int32 NewTotalMoney = GetCurrentMoney() + ActrualReward;
	SetCurrentMoney(NewTotalMoney);
}

// 유물 관련 함수
void UInventoryComponent::EquipRelic()
{

}

void UInventoryComponent::UnequipRelic()
{

}
