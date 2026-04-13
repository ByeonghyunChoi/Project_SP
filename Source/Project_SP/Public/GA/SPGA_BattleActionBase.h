#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Manager/SPGASBattleTypes.h"
#include "SPGA_BattleActionBase.generated.h"

UCLASS()
class PROJECT_SP_API USPGA_BattleActionBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USPGA_BattleActionBase();
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Combat|TimeInterference")
	bool ConsumeTimeInterferenceStack();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	FGameplayEventData CachedEventData;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	int32 CooldownTurns = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Damage")
	float DefaultDamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Damage")
	float SecondaryDamageMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Targeting")
	ETargetingType SkillTargetingType = ETargetingType::Single;

	// 해당 스킬이 터트릴 이펙트 태그
	UPROPERTY(EditDefaultsOnly, Category = "Combat | VFX")
	FGameplayTag HitVFXTag;

public:
	FGameplayTag GetCooldownTag() const;

protected:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamageToTarget(AActor* TargetActor, float DamageMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyTurnBasedCooldown();

	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetSingleTarget() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	TArray<AActor*> GetAllEnemies() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	TArray<AActor*> GetSecondaryTargets(AActor* PrimaryTarget) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetRandomEnemy() const;

	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Category = "Combat|TimeInterference")
	void ActivateTimeInterference(int32 ExtraTurns = 2);

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Camera")
	TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;
};