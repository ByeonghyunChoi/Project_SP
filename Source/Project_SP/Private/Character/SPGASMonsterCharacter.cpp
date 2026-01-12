#include "Character/SPGASMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"



ASPGASMonsterCharacter::ASPGASMonsterCharacter()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AttributeSet = CreateDefaultSubobject<USPGASAttributeSet>(TEXT("AttributeSet"));
}

void ASPGASMonsterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ASC->InitAbilityActorInfo(this, this);
}

