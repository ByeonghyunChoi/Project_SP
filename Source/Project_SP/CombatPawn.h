// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.h"
#include "CombatPawn.generated.h"

UCLASS()
class PROJECT_SP_API ACombatPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatPawn();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CombatData")
	virtual UCharacterBase* GetCombatData() const;

	// 기본 공격 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Action")
	virtual void Attack(ACombatPawn* Target); 

	// 스킬 사용 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Action")
	virtual void UseSkill(int32 SkillID, ACombatPawn* Target); 

	// 데미지를 받는 함수 (피격 반응 처리)
	UFUNCTION(BlueprintCallable, Category = "Combat|Action")
	virtual void ReceiveDamage(float DamageAmount);

	// 특정 진영의 모든 살아있는 CombatPawn을 반환 (AI 타겟 선택용)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Targeting")
	TArray<ACombatPawn*> GetAllAliveCombatantsOfFaction(EFaction TargetFaction) const;

protected:
	//데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"), Instanced = "true")
	UCharacterBase* CombatData;

};
