// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Oparts_CrystalSkull.generated.h"


UCLASS()
class PROJECT_SP_API UGA_Oparts_CrystalSkull : public UGameplayAbility
{
	GENERATED_BODY()
	
public:

	UGA_Oparts_CrystalSkull();

	// 스킬이 부여되고 실행될 때 호출되는 함수
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 치명타 이벤트(GameplayEvent)를 감지했을 때 실행될 콜백 함수
	UFUNCTION()
	void OnCriticalHitReceived(FGameplayEventData Payload);

	// 게이지가 100 찼을 때 플레이어에게 부여할 '다음 스킬 데미지 100% 증가' 버프 (블루프린트에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "CrystalSkull")
	TSubclassOf<class UGameplayEffect> SkillDamageBuffClass;

	// 현재 수정해골 게이지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrystalSkull")
	float CurrentGauge;
};
