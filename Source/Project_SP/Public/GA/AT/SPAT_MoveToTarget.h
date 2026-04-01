// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SPAT_MoveToTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMoveAndMontageDelegate);

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPAT_MoveToTarget : public UAbilityTask
{
	GENERATED_BODY()

public:
	USPAT_MoveToTarget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FMoveAndMontageDelegate OnCompleted;

	//생성기
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USPAT_MoveToTarget* MoveAndMontage(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		AActor* TargetActor,           // 돌진할 타겟 (없으면 아래 Transform으로 감)
		FTransform TargetTransform,    // 복귀할 좌표 (TargetActor가 비어있을 때 사용)
		float DistanceOffset = 50.0f, // 타겟 앞 몇 거리에서 멈출지
		float MoveDuration = 0.2f,     // 이동하는 데 걸리는 시간
		UAnimMontage* MontageToPlay = nullptr // 이동하면서 틀 애니메이션
	);

	// 태스크 생명주기 함수들
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

private:
	// 내부 계산용 변수들
	UPROPERTY()
	AActor* TargetActor;

	FTransform TargetTransform;
	float DistanceOffset;
	float MoveDuration;

	UPROPERTY()
	UAnimMontage* MontageToPlay;

	float ElapsedTime;
	FVector StartLocation;
	FRotator StartRotation;
	FVector DestLocation;
	FRotator DestRotation;
	bool bIsMoving;

	// 🌟 적 앞 거리 수학 계산을 C++이 대신 해줍니다.
	void CalculateDestination();
	
};
