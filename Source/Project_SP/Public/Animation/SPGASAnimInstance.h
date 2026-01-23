// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SPGASAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGASAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	USPGASAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	//기본 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class ASPGASCharacterBase> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class UCharacterMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	uint8 bIsIdle : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	uint8 bIsFalling : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovingThreshold;

	// 상태 데이터
	UPROPERTY(BlueprintReadOnly, Category = "GAS State")
	bool bIsInBattle; // 전투 모드 여부

	UPROPERTY(BlueprintReadOnly, Category = "GAS State")
	bool bIsMyTurn;   // 현재 자신의 턴인지 여부

	UPROPERTY(BlueprintReadOnly, Category = "GAS State")
	bool bIsDead;     // 사망 상태 여부
};
