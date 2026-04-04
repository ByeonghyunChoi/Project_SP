// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_OnHitStatusBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_OnHitStatusBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Relic_OnHitStatusBase();

    // 어빌리티가 시작될 때 호출 (여기서 이벤트 감시 시작)
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    // 타격 이벤트를 받았을 때 실행될 함수
    UFUNCTION()
    void OnHitEventReceived(FGameplayEventData Payload);

    // 에디터에서 설정할 풍화 디버프 GE
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TSubclassOf<class UGameplayEffect> TargetStatusGEClass;

    // 발동 확률 (30)
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Stats")
    float TriggerChance = 30.0f;
	
};
