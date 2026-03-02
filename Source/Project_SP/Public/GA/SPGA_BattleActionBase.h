// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_BattleActionBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_BattleActionBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	USPGA_BattleActionBase();
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintCallable, Category = "Combat|TimeInterference")
	bool ConsumeTimeInterferenceStack();

protected:
	/** 데미지 적용용 GE (GE_BattleDamage) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 쿨타임 적용용 GE (GE_TurnBasedCooldown - Infinite, Stack 방식) */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

	/** 이 스킬의 쿨타임 턴 수 (0이면 쿨타임 없음) */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	int32 CooldownTurns = 0;

	/** 쿨타임 동안 부여할 태그 (예: Cooldown.Weapon.Fenrir.Skill) */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FGameplayTag CooldownTag;

	/**
	 * 타겟에게 데미지를 입힙니다.
	 * @param TargetActor   맞을 대상
	 * @param DamageMultiplier 스킬 계수 (예: 1.0 = 100%, 2.2 = 220%)
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamageToTarget(AActor* TargetActor, float DamageMultiplier);

	/**
	 * 턴제 쿨타임을 적용합니다.
	 * CooldownTurns 만큼 스택을 쌓고, CooldownTag를 부여합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyTurnBasedCooldown();

	/** 플레이어 컨트롤러가 선택한 단일 타겟 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetSingleTarget() const;

	/** 모든 적(Enemy 태그) 가져오기 (광역기용) */
	UFUNCTION(BlueprintPure, Category = "Combat")
	TArray<AActor*> GetAllEnemies() const;

	// 주 타겟을 제외한 나머지 보조 타겟들만 가져오기
	UFUNCTION(BlueprintPure, Category = "Combat")
	TArray<AActor*> GetSecondaryTargets(AActor* PrimaryTarget) const;

	/** 랜덤한 적 1명 가져오기 (바운스용) */
	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetRandomEnemy() const;
};
