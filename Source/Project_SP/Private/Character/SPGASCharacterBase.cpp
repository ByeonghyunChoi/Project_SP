// CombatPawn.cpp

#include "Character/SPGASCharacterBase.h"

ASPGASCharacterBase::ASPGASCharacterBase()
{
    
}

UAbilitySystemComponent* ASPGASCharacterBase::GetAbilitySystemComponent() const
{
    return ASC;
}

