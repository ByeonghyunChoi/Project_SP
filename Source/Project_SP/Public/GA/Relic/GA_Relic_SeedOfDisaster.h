// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_SeedOfDisaster.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_SeedOfDisaster : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_Relic_SeedOfDisaster();
    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    UFUNCTION()
    void OnNormalAttackHitReceived(FGameplayEventData Payload);

    //전투 종료 방송을 들었을 때 실행할 함수
    UFUNCTION()
    void OnBattleEndReceived(FGameplayEventData Payload);

    // 에디터에서 방금 만든 5스택 GE(GE_Relic_SeedBuff)를 할당할 변수
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    TSubclassOf<class UGameplayEffect> SeedStackGEClass;

};
