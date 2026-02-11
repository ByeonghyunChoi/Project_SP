// CombatPawn.cpp

#include "Character/SPGASCharacterBase.h"
#include "AbilitySystemComponent.h"


ASPGASCharacterBase::ASPGASCharacterBase()
{
    ASC = nullptr;
    AttributeSet = nullptr;
}

UAbilitySystemComponent* ASPGASCharacterBase::GetAbilitySystemComponent() const
{
    return ASC;
}

