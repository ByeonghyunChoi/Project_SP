// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_MonsterAttackBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_MonsterAttackBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USPGA_MonsterAttackBase();

protected:
	// 어빌리티 시작 시 패링 이벤트 리스너(WaitGameplayEvent)를 켭니다.
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 플레이어에게 패링 당했을 때 호출되는 함수! (블루프린트에서 연출 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnParried();

	UFUNCTION()
	void OnParriedEventCallback(FGameplayEventData Payload);
	
};
