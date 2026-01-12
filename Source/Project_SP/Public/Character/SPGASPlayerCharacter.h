#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "SPGASPlayerCharacter.generated.h"

UCLASS()
class PROJECT_SP_API ASPGASPlayerCharacter : public ASPGASCharacterBase
{
	GENERATED_BODY()

public:
	ASPGASPlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;

};