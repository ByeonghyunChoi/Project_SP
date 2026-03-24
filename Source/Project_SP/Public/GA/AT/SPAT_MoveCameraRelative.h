// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SPAT_MoveCameraRelative.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMoveCameraDelegate);

UENUM(BlueprintType)
enum class ECameraEasingType : uint8
{
	Linear		UMETA(DisplayName = "일정한 속도 (Linear)"),
	EaseIn		UMETA(DisplayName = "느리게 출발 ➡️ 훅! 도착 (Ease-In)"),
	EaseOut		UMETA(DisplayName = "팍! 출발 ➡️ 스무스 도착 (Ease-Out)"),
	EaseInOut	UMETA(DisplayName = "느리게 ➡️ 최고속도 ➡️ 스무스 (Ease-InOut)")
};

UCLASS()
class PROJECT_SP_API USPAT_MoveCameraRelative : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	USPAT_MoveCameraRelative(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FMoveCameraDelegate OnCompleted;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USPAT_MoveCameraRelative* MoveCameraRelative(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		USceneComponent* TargetComponent,
		FVector TargetRelativeLocation,
		FRotator TargetRelativeRotation,
		float Duration,
		// 🌟 블루프린트 핀에 추가! (기본값은 타격감이 좋은 EaseOut)
		ECameraEasingType EasingType = ECameraEasingType::EaseOut
	);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

private:
	UPROPERTY()
	USceneComponent* TargetComponent;

	FVector StartLocation;
	FRotator StartRotation;
	FVector TargetLocation;
	FRotator TargetRotation;

	float Duration;
	float ElapsedTime;
	bool bIsMoving;

	// 🌟 연출 방식을 기억할 변수 추가
	ECameraEasingType EasingType;
};
