// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/SPGA_CombatActionBase.h"
#include "SPGA_MonsterAttackBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_MonsterAttackBase : public USPGA_CombatActionBase
{
	GENERATED_BODY()

public:
	USPGA_MonsterAttackBase();

protected:
	virtual void SetupActionEventListeners() override;

	// 플레이어에게 패링 당했을 때 호출되는 함수! (블루프린트에서 연출 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnParried();

	UFUNCTION()
	void OnParriedEventCallback(FGameplayEventData Payload);

	// 몬스터가 데미지를 줄 때 사용할 Gameplay Effect (블루프린트에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// 데미지 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Damage")
	float DamageMultiplier = 1.0f;

	// 충돌한 타겟(플레이어)에게 데미지 이펙트를 적용하는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	void ApplyDamageToTarget(AActor* TargetActor, float Damage);

	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);
	
};
