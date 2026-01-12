// CombatPawn.cpp

#include "Character/SPGASCharacterBase.h"

ASPGASCharacterBase::ASPGASCharacterBase()
{
    ASC = nullptr;
    AttributeSet = nullptr;
}

UAbilitySystemComponent* ASPGASCharacterBase::GetAbilitySystemComponent() const
{
    return ASC;
}

