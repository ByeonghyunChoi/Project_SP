#include "Character/SPGASMonsterCharacter.h"
#include "Character/SPGASMonsterState.h"
#include "AbilitySystemComponent.h"




ASPGASMonsterCharacter::ASPGASMonsterCharacter()
{
}

void ASPGASMonsterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASPGASMonsterState* SPGAS = GetPlayerState<ASPGASMonsterState>();
	if (SPGAS)
	{
		ASC = SPGAS->GetAbilitySystemComponent();
		AttributeSet = SPGAS->GetAttributeSet();
		ASC->InitAbilityActorInfo(SPGAS, this);
	}
}


