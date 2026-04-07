// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Relic_AddGold.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UGA_Relic_AddGold : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_Relic_AddGold();

    // 유물이 플레이어의 주머니(ASC)에 들어오는 순간 즉시 호출되는 함수!
    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
    // 지급할 골드량 (블루프린트 디폴트에서 수정 가능)
    UPROPERTY(EditDefaultsOnly, Category = "Relic | Effect")
    int32 GoldAmountToAdd = 100;

};
