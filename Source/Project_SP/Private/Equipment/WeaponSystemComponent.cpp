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
	OwnerPlayer = Cast<APlayerCharacter>(GetOwner());

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
	if (!bIsParryWindowOpen || !OwnerPlayer || !ParryAttacker.IsValid()) return false;

	EParryResult Result;
	// 내가 바꾸려는 무기 타입이 공격하는 몬스터의 약점과 일치하는가?
	if (WeaponTypeToSwitch == ParryAttacker->WeaknessType)
	{
		Result = EParryResult::Success;
		UE_LOG(LogTemp, Warning, TEXT("Parry SUCCESS!"));
		EquipWeapon(WeaponTypeToSwitch); // 무기 교체
		// 스위치 스킬로 반격
		if (CurrentWeapon && CurrentWeapon->SwitchSkillActionID != NAME_None)
		{
			TArray<ACombatPawn*> CounterTarget;
			CounterTarget.Add(ParryAttacker.Get());
			OwnerPlayer->PlayerSelectAction(CurrentWeapon->SwitchSkillActionID);
			OwnerPlayer->PlayerSelectAction(CurrentWeapon->SwitchSkillActionID);
		}
	}
	else
	{
		Result = EParryResult::PartialSuccess;
		UE_LOG(LogTemp, Log, TEXT("Parry Partial Success (Guard)."));
		EquipWeapon(WeaponTypeToSwitch); // 무기 교체는 동일하게 함
	}

	bIsParryWindowOpen = false; // 입력 기회는 한 번뿐

	// 결과를 BattleManager에게 브로드캐스트
	if (OwnerPlayer->GameEventComponent)
	{
		OwnerPlayer->GameEventComponent->BroadcastParryAttempted(ParryAttacker.Get(), OwnerPlayer, Result);
	}
	return true;
}