// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SPAT_LerpCamera.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLerpFloatDelegate, float, CurrentValue);

UCLASS()
class PROJECT_SP_API USPAT_LerpCamera : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	USPAT_LerpCamera(const FObjectInitializer& ObjectInitializer);

	// 타임라인의 Update 핀 역할 (매 프레임 실행)
	UPROPERTY(BlueprintAssignable)
	FLerpFloatDelegate OnUpdate;

	// 타임라인의 Finished 핀 역할 (끝났을 때 한 번 실행)
	UPROPERTY(BlueprintAssignable)
	FLerpFloatDelegate OnCompleted;

	// 노드 생성기
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static USPAT_LerpCamera* LerpFloatOverTime(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		float StartValue, // 시작 숫자 (예: 800)
		float EndValue,   // 끝나는 숫자 (예: 300)
		float Duration    // 걸리는 시간 (예: 0.15)
	);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

private:
	float StartValue;
	float EndValue;
	float Duration;
	float ElapsedTime;
	bool bIsLerping;
};
