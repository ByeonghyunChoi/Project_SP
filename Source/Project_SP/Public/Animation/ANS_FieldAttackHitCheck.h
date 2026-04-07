// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_FieldAttackHitCheck.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UANS_FieldAttackHitCheck : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

private:
	// 판정 범위
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float AttackRadius = 40.0f;

	// 판정을 추적한 소켓의 이름
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponSocketName = FName("WeaponSocket");

	// 디버그 온, 오프
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bShowDebug = false;
};
