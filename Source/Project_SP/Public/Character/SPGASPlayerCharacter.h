#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "GameplayTagContainer.h"
#include "Data/Asset/WeaponAbilityData.h"
#include "Manager/SPGASBattleTypes.h"
#include "SPGASPlayerCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCameraProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetArmLength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RelativeRotation = FRotator(-45.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableLag = true;

	//카메라 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CameraRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CameraRelativeRotation = FRotator::ZeroRotator;
};


UCLASS()
class PROJECT_SP_API ASPGASPlayerCharacter : public ASPGASCharacterBase
{
	GENERATED_BODY()

public:
	ASPGASPlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	void ActivateCombatAbility(FGameplayTag WeaponTag, ESelectedActionType ActionType, AActor* TargetActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> CurrentCombatTarget;
protected:
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditAnywhere, Category = "GAS | Field")
	TMap<FGameplayTag, TSubclassOf<class UGameplayAbility>> FieldInputAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Field")
	TArray<TSubclassOf<class UGameplayAbility>> FieldPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Battle")
	TMap<FGameplayTag, TSubclassOf<class UGameplayAbility>> BattleInputAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Battle")
	TArray<TSubclassOf<class UGameplayAbility>> BattlePassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS | Battle")
	TMap<FGameplayTag, TObjectPtr<UWeaponAbilityData>> WeaponConfigs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	// 실제 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

	void GiveAbilities();

	void GiveWeaponAbilities();

	//임시 함수 나중에 제거
	void OnGameplayEffectApplied(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class USPInteractionComponent> InteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Camera | Profile")
	FCameraProfile FieldCameraSetting;

	UPROPERTY(EditDefaultsOnly, Category = "Camera | Profile")
	FCameraProfile CombatCameraSetting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> WeaponWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> ActionWidgetComponent;

public:
	void SetCameraProfile(const FCameraProfile& Profile);

	// Getter
	const FCameraProfile& GetFieldCameraProfile() const { return FieldCameraSetting; }
	const FCameraProfile& GetCombatCameraProfile() const { return CombatCameraSetting; }
	TObjectPtr<class UWidgetComponent> GetWeaponWidgetComponent() { return WeaponWidgetComponent; }
	TObjectPtr<class UWidgetComponent> GetActionWidgetComponent() { return ActionWidgetComponent; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	ETargetingType GetTargetingType(FGameplayTag WeaponTag, ESelectedActionType ActionType) const;
	TObjectPtr<UWeaponAbilityData> GetWeaponData(FGameplayTag WeaponTag) const;
};