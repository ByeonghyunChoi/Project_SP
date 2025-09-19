// Equipment/WeaponSystemComponent.cpp

#include "Component/WeaponSystemComponent.h"
#include "Data/WeaponData.h"
#include "Component/ActionComponent.h"
#include "GameFramework/Actor.h"

void UWeaponSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    ActionComponent = GetOwner()->FindComponentByClass<UActionComponent>();
}

void UWeaponSystemComponent::AcquireWeapon(EDamageType NewWeaponType)
{
    if (PossessedWeaponTypes.Contains(NewWeaponType)) return;

    PossessedWeaponTypes.Add(NewWeaponType);
    OnWeaponAcquired.Broadcast(NewWeaponType);
    UE_LOG(LogTemp, Warning, TEXT("%s 무기를 획득했습니다!"), *UEnum::GetValueAsString(NewWeaponType));

    if (PossessedWeaponTypes.Num() == 1)
    {
        SwitchWeapon(NewWeaponType);
    }
}

void UWeaponSystemComponent::SwitchWeapon(EDamageType WeaponTypeToSwitch)
{
    if (!ActionComponent || !PossessedWeaponTypes.Contains(WeaponTypeToSwitch)) return;
    if (CurrentWeapon && CurrentWeapon->WeaponType == WeaponTypeToSwitch) return;

    if (CurrentWeapon)
    {
        ActionComponent->RemoveAction(CurrentWeapon->BasicAttackActionID);
        ActionComponent->RemoveAction(CurrentWeapon->SpecialSkillActionID);
        ActionComponent->RemoveAction(CurrentWeapon->ParrySkillActionID);
    }

    if (WeaponDataAssets.Contains(WeaponTypeToSwitch))
    {
        CurrentWeapon = WeaponDataAssets[WeaponTypeToSwitch];
        if (CurrentWeapon)
        {
            ActionComponent->GrantAction(CurrentWeapon->BasicAttackActionID);
            ActionComponent->GrantAction(CurrentWeapon->SpecialSkillActionID);
            ActionComponent->GrantAction(CurrentWeapon->ParrySkillActionID);

            OnWeaponSwitched.Broadcast(CurrentWeapon);
            UE_LOG(LogTemp, Log, TEXT("%s 무기로 교체!"), *UEnum::GetValueAsString(WeaponTypeToSwitch));
        }
    }
}

bool UWeaponSystemComponent::HasWeapon(EDamageType WeaponType) const
{
    return PossessedWeaponTypes.Contains(WeaponType);
}