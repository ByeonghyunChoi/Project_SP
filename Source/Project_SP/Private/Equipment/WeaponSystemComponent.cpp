#include "Equipment/WeaponSystemComponent.h"
#include "Core/BattleManager.h"
#include "Equipment/Weapon.h"
#include "Combat/PlayerCharacter.h"
#include "Event/GameEventComponent.h"
#include "Combat/CharacterStatsComponent.h"
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

    // 1. 스탯 컴포넌트를 가져와 SP가 100 이상인지 확인
    UCharacterStatsComponent* StatsComp = OwnerPlayer->GetStatsComponent();
    if (!StatsComp || StatsComp->GetCurrentSP() < 100.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough SP to parry. Required: 100, Have: %f"), StatsComp ? StatsComp->GetCurrentSP() : 0.0f);
        return false;
    }

    // 2. SP 100 소모
    StatsComp->ModifySP(-100.0f);
    UE_LOG(LogTemp, Log, TEXT("Used 100 SP for parry. Remaining SP: %f"), StatsComp->GetCurrentSP());


    EParryResult Result;
    if (WeaponTypeToSwitch == ParryAttacker->WeaknessType)
    {
        Result = EParryResult::Success;
        UE_LOG(LogTemp, Warning, TEXT("Parry SUCCESS!"));

        // 3. 패링 성공 시 SP 50 회복
        StatsComp->ModifySP(50.0f);
        UE_LOG(LogTemp, Log, TEXT("Parry success! Recovered 50 SP. Current SP: %f"), StatsComp->GetCurrentSP());

        EquipWeapon(WeaponTypeToSwitch);

        if (CurrentWeapon && CurrentWeapon->SwitchSkillActionID != NAME_None)
        {
            TArray<ACombatPawn*> CounterTarget;
            CounterTarget.Add(ParryAttacker.Get());
            // PlayerSelectAction은 두 번 호출하여 즉시 실행하도록 되어있습니다.
            OwnerPlayer->PlayerSelectAction(CurrentWeapon->SwitchSkillActionID);
            OwnerPlayer->PlayerSelectAction(CurrentWeapon->SwitchSkillActionID);
        }
    }
    else
    {
        Result = EParryResult::PartialSuccess;
        UE_LOG(LogTemp, Log, TEXT("Parry Partial Success (Guard)."));
        EquipWeapon(WeaponTypeToSwitch);
    }

    bIsParryWindowOpen = false;

    if (OwnerPlayer->GameEventComponent)
    {
        OwnerPlayer->GameEventComponent->BroadcastParryAttempted(ParryAttacker.Get(), OwnerPlayer, Result);
    }
    return true;
}