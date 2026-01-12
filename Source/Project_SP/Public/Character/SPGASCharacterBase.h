// CombatPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "SPGASCharacterBase.generated.h"


UCLASS(Abstract) 
class PROJECT_SP_API ASPGASCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ASPGASCharacterBase();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "GAS")
    TObjectPtr<class UAbilitySystemComponent> ASC;

    UPROPERTY(EditAnywhere, Category = "GAS")
    TObjectPtr<class USPGASAttributeSet> AttributeSet;

public:
    FORCEINLINE class USPGASAttributeSet* GetAttributeSet() const { return AttributeSet; }
};