// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/AT/SPAT_LerpCamera.h"

USPAT_LerpCamera::USPAT_LerpCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bIsLerping = false;
	ElapsedTime = 0.0f;
}

USPAT_LerpCamera* USPAT_LerpCamera::LerpFloatOverTime(UGameplayAbility* OwningAbility, FName TaskInstanceName, float StartValue, float EndValue, float Duration)
{
	USPAT_LerpCamera* MyObj = NewAbilityTask<USPAT_LerpCamera>(OwningAbility, TaskInstanceName);
	MyObj->StartValue = StartValue;
	MyObj->EndValue = EndValue;
	MyObj->Duration = FMath::Max(0.01f, Duration); 
	return MyObj;
}

void USPAT_LerpCamera::Activate()
{
	Super::Activate();
	bIsLerping = true;
}

void USPAT_LerpCamera::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsLerping)
	{
		ElapsedTime += DeltaTime;

		float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);

		// 스무스하게 숫자가 변하도록 EaseInOut 적용!
		float SmoothAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
		float CurrentLerpValue = FMath::Lerp(StartValue, EndValue, SmoothAlpha);

		// 블루프린트로 현재 값(CurrentValue) 쏴주기!
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnUpdate.Broadcast(CurrentLerpValue);
		}

		// 다 끝났으면 Completed 쏴주고 소멸!
		if (Alpha >= 1.0f)
		{
			bIsLerping = false;
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnCompleted.Broadcast(EndValue);
			}
			EndTask();
		}
	}
}
