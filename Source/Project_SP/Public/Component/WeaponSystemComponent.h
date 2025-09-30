#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ActionData.h"
#include "WeaponSystemComponent.generated.h"

class UWeaponData;
class UActionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, UWeaponData*, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAcquired, EDamageType, AcquiredWeaponType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UWeaponSystemComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
    TMap<EDamageType, TObjectPtr<UWeaponData>> WeaponDataAssets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
    TSet<EDamageType> PossessedWeaponTypes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
    TObjectPtr<UWeaponData> CurrentWeapon;

    UPROPERTY()
    TObjectPtr<UActionComponent> ActionComponent;

public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void AcquireWeapon(EDamageType NewWeaponType);

    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void SwitchWeapon(EDamageType WeaponTypeToSwitch);

    UFUNCTION(BlueprintPure, Category = "Weapons")
    bool HasWeapon(EDamageType WeaponType) const;

    UFUNCTION(BlueprintPure, Category = "Weapons")
    UWeaponData* GetCurrentWeapon() const { return CurrentWeapon; }

    UPROPERTY(BlueprintAssignable)
    FOnWeaponSwitched OnWeaponSwitched;

    UPROPERTY(BlueprintAssignable)
    FOnWeaponAcquired OnWeaponAcquired;
};