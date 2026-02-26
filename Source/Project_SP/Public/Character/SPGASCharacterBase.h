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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<class USPStatusEffectComponent> StatusEffectComponent;

public:
    FORCEINLINE class USPGASAttributeSet* GetAttributeSet() const { return AttributeSet; }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<class UAnimMontage> FieldAttackMontage;

    //턴을 종료하고 GameMode에게 알림
    UFUNCTION(BlueprintCallable, Category = "GAS | Turn")
    virtual void FinishTurn();

    //턴 시작 시 쿨타임 감소
    UFUNCTION(BlueprintCallable, Category = "GAS | Turn")
    virtual void ReduceCooldowns();

    FORCEINLINE class USPStatusEffectComponent* GetStatusEffectComponent() const { return StatusEffectComponent; }
};