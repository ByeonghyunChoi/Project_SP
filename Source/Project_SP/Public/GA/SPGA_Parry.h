// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SPGA_Parry.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGA_Parry : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	// 무기와 약점이 일치하는지 검사
	UFUNCTION(BlueprintCallable, Category = "Parry")
	bool CheckWeaponMatch(class ASPGASMonsterCharacter* TargetMonster);

	// 반격 조건이 모두 만족하는지 검사
	UFUNCTION(BlueprintCallable, Category = "Parry")
	bool CheckCounterConditions();

	// 패링 성공 시 몬스터에게 "너 패링당했어!" 라고 이벤트 쏘기
	UFUNCTION(BlueprintCallable, Category = "Parry")
	void SendParriedEventToMonster(AActor* TargetMonster);

protected:
	// 실제로 나갈 반격 스킬(GA)의 태그 (쿨타임 검사용)
	UPROPERTY(EditDefaultsOnly, Category = "Parry")
	FGameplayTag CounterSkillTag;
};
