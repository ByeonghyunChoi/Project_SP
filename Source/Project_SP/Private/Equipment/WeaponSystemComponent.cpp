// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/WeaponSystemComponent.h"

// Sets default values for this component's properties
UWeaponSystemComponent::UWeaponSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
	// ...
}


// Called when the game starts
void UWeaponSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentWeapon = *Weapons.Find(EDamageType::Fenrir);
	// ...
	
}

void UWeaponSystemComponent::EquipWeapon(EDamageType WeaponTypeToEquip)
{
	if (Weapons.Contains(WeaponTypeToEquip))
	{
		CurrentWeapon = Weapons[WeaponTypeToEquip];
		UE_LOG(LogTemp, Log, TEXT("Weapon Switched to: %s"), *UEnum::GetValueAsString(WeaponTypeToEquip));

		// --- 이 줄을 추가하여 신호를 보냅니다 ---
		OnWeaponEquipped.Broadcast(CurrentWeapon);
	}
}

bool UWeaponSystemComponent::AttemptParry(EDamageType WeaponTypeToSwitch)
{
	return false;
}


