// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/WeaponSystemComponent.h"
#include "Core/BattleManager.h"
#include "Equipment/Weapon.h"
#include "Combat/PlayerCharacter.h"
#include "Event/GameEventComponent.h"
#include "Kismet/GameplayStatics.h"

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

	ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
	if (BattleManager)
	{
		// 타이밍 문제를 피하기 위해, BattleManager가 준비된 후 바인딩하는 것이 더 안전할 수 있습니다.
		// 지금은 간단하게 BeginPlay에서 처리합니다.
		for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
		{
			if (Combatant && Combatant->GetFaction() == EFaction::Enemy)
			{
				if (Combatant->GameEventComponent)
				{
					Combatant->GameEventComponent->OnParryWindowChanged.AddDynamic(this, &UWeaponSystemComponent::HandleParryWindowChanged);
				}
			}
		}
	}
	if (!CurrentWeapon && Weapons.Contains(EDamageType::Fenrir))
	{
		// 'Fenrir'를 기본 무기로 장착시킨다.
		EquipWeapon(EDamageType::Fenrir);
	}
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

void UWeaponSystemComponent::HandleParryWindowChanged(ACombatPawn* Attacker, EDamageType AttackType, bool bIsWindowOpen)
{
	bIsParryWindowOpen = bIsWindowOpen;
	if (bIsWindowOpen)
	{
		ParryAttacker = Attacker;
		ParryAttackType = AttackType;
		UE_LOG(LogTemp, Log, TEXT("Parry window OPENED. Attacker: %s, Type: %s"), *Attacker->GetName(), *UEnum::GetValueAsString(AttackType));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Parry window CLOSED."));
	}
}

bool UWeaponSystemComponent::AttemptParry(EDamageType WeaponTypeToSwitch)
{
	// 1. 패링 창이 열려있는지, 공격하는 몬스터가 유효한지 확인
	if (!bIsParryWindowOpen || !OwnerPlayer || !ParryAttacker.IsValid())
	{
		return false;
	}

	// 2. "내가 바꾸려는 무기 타입"이 "공격하는 몬스터의 약점 타입"과 일치하는지 확인
	if (WeaponTypeToSwitch == ParryAttacker->WeaknessType)
	{
		UE_LOG(LogTemp, Warning, TEXT("Parry SUCCESS against %s's weakness!"), *ParryAttacker->GetName());
		bIsParryWindowOpen = false; // 한 번 성공하면 창을 닫아 중복 입력 방지

		// 3. 무기 교체 (이미 같은 무기라도 EquipWeapon을 호출하여 UI 신호를 보낼 수 있음)
		EquipWeapon(WeaponTypeToSwitch);

		// 4. 스위치 스킬로 반격
		if (CurrentWeapon && CurrentWeapon->SwitchSkillActionID != NAME_None)
		{
			TArray<ACombatPawn*> CounterAttackTarget;
			CounterAttackTarget.Add(ParryAttacker.Get());

			// 플레이어의 공격 로직을 그대로 사용하여 반격 실행
			OwnerPlayer->PlayerSelectAction(CurrentWeapon->SwitchSkillActionID); // 선택과 즉시 실행
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Parry FAILED. %s is not the weakness of %s."), *UEnum::GetValueAsString(WeaponTypeToSwitch), *ParryAttacker->GetName());
		return false;
	}
	return false;
}


