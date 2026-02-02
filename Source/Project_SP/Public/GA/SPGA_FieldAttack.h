// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_FieldAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_FieldAttack : public UGameplayAbility
{
	GENERATED_BODY()
public:
	USPGA_FieldAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	// 공격 판정 이벤트
	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);

	// 몽타주가 끝났을 때(혹은 캔슬됐을 때) 실행
	UFUNCTION()
	void OnMontageEnded();

	// 전투 진입 여부 결정 함수 (핵심 로직)
	void ResolveBattleEncounter(AActor* Attacker, AActor* Victim);
};
