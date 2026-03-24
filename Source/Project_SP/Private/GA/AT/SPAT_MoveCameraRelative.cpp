// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/AT/SPAT_MoveCameraRelative.h"

USPAT_MoveCameraRelative::USPAT_MoveCameraRelative(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bIsMoving = false;
	ElapsedTime = 0.0f;
}

USPAT_MoveCameraRelative* USPAT_MoveCameraRelative::MoveCameraRelative(UGameplayAbility* OwningAbility, FName TaskInstanceName, USceneComponent* TargetComponent, FVector TargetRelativeLocation, FRotator TargetRelativeRotation, float Duration, ECameraEasingType EasingType)
{
	USPAT_MoveCameraRelative* MyObj = NewAbilityTask<USPAT_MoveCameraRelative>(OwningAbility, TaskInstanceName);
	MyObj->TargetComponent = TargetComponent;
	MyObj->TargetLocation = TargetRelativeLocation;
	MyObj->TargetRotation = TargetRelativeRotation;
	MyObj->Duration = FMath::Max(0.01f, Duration);
	MyObj->EasingType = EasingType; // 🌟 핀에서 받은 연출 방식 저장
	return MyObj;
}

void USPAT_MoveCameraRelative::Activate()
{
	Super::Activate();
	if (!TargetComponent)
	{
		EndTask();
		return;
	}
	StartLocation = TargetComponent->GetRelativeLocation();
	StartRotation = TargetComponent->GetRelativeRotation();
	bIsMoving = true;
}

void USPAT_MoveCameraRelative::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsMoving && TargetComponent)
	{
		ElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);

		// 🌟 선택한 연출 방식에 따라 수학 공식을 다르게 적용합니다!
		float SmoothAlpha = Alpha;
		switch (EasingType)
		{
		case ECameraEasingType::EaseIn:
			SmoothAlpha = FMath::InterpEaseIn(0.0f, 1.0f, Alpha, 2.0f); // 수치 2.0f는 강도입니다.
			break;
		case ECameraEasingType::EaseOut:
			SmoothAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
			break;
		case ECameraEasingType::EaseInOut:
			SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
			break;
		case ECameraEasingType::Linear:
		default:
			SmoothAlpha = Alpha; // 일정한 속도
			break;
		}

		FVector NewLoc = FMath::Lerp(StartLocation, TargetLocation, SmoothAlpha);
		FRotator NewRot = FMath::Lerp(StartRotation, TargetRotation, SmoothAlpha);

		TargetComponent->SetRelativeLocationAndRotation(NewLoc, NewRot);

		if (Alpha >= 1.0f)
		{
			bIsMoving = false;
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnCompleted.Broadcast();
			}
			EndTask();
		}
	}
}