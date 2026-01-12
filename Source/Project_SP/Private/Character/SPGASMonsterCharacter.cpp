#include "Character/SPGASMonsterCharacter.h"
#include "AbilitySystemComponent.h"



ASPGASMonsterCharacter::ASPGASMonsterCharacter()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
}

void ASPGASMonsterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ASC->InitAbilityActorInfo(this, this);
}

