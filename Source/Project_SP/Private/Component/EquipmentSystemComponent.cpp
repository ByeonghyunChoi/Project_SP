// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EquipmentSystemComponent.h"
#include "Items/OpartsBase.h"
#include "Items/CrystalSkullOparts.h"
#include "Items/JadeClockOparts.h"
#include "Items/GoldBugOparts.h"

UEquipmentSystemComponent::UEquipmentSystemComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

}

// PlayerCharacter에서 시작 시 포인터를 설정하는 함수
void UEquipmentSystemComponent::InitializeOpartsPointers(UCrystalSkullOparts* Skull, UJadeClockOparts* Clock, UGoldBugOparts* Bug)
{
	CrystalSkull = Skull;
	JadeClock = Clock;
	GoldBug = Bug;
}

void UEquipmentSystemComponent::SetOpartsActiveState(UOpartsBase* Oparts, bool bActive)
{
	if (Oparts)
	{
		// 1. 컴포넌트의 전체 활성화 상태 변경
		Oparts->SetActive(bActive);

		// 2. 오파츠의 장착/해제 로직 호출 (스탯 적용/제거)
		if (bActive)
		{
			Oparts->OnEquip(GetOwner());
		}
		else
		{
			Oparts->OnUnequip(GetOwner());
		}
	}
}

void UEquipmentSystemComponent::ActivateCrystalSkull()
{
	ActiveOpartsIndex = 0;
	// 수정 해골 활성화
	SetOpartsActiveState(CrystalSkull, true);

	// 나머지 두 개 비활성화
	SetOpartsActiveState(JadeClock, false);
	SetOpartsActiveState(GoldBug, false);

	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}
}

void UEquipmentSystemComponent::ActivateJadeClock()
{
	ActiveOpartsIndex = 1;
	SetOpartsActiveState(JadeClock, true);

	// 나머지 두 개 비활성화
	SetOpartsActiveState(CrystalSkull, false);
	SetOpartsActiveState(GoldBug, false);

	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}
}

void UEquipmentSystemComponent::ActivateGoldBug()
{
	ActiveOpartsIndex = 2;
	SetOpartsActiveState(GoldBug, true);

	// 나머지 두 개 비활성화
	SetOpartsActiveState(CrystalSkull, false);
	SetOpartsActiveState(JadeClock, false);

	// CrystalSkull 상태 확인
	if (CrystalSkull)
	{
		FString State = CrystalSkull->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("CrystalSkull Oparts State: %s"), *State);
	}

	// JadeClock 상태 확인
	if (JadeClock)
	{
		FString State = JadeClock->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("JadeClock Oparts State: %s"), *State);
	}

	// GoldBug 상태 확인
	if (GoldBug)
	{
		FString State = GoldBug->IsActive() ? TEXT("활성화됨 (Active)") : TEXT("비활성화됨 (Inactive)");
		UE_LOG(LogTemp, Warning, TEXT("GoldBug Oparts State: %s"), *State);
	}
}

void UEquipmentSystemComponent::ActivateLastOparts()
{
	switch (ActiveOpartsIndex)
	{
	case 0:
		ActivateCrystalSkull();
		break;
	case 1:
		ActivateJadeClock();
		break;
	case 2:
		ActivateGoldBug();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("No previously active Oparts to activate."));
		break;
	}
}



