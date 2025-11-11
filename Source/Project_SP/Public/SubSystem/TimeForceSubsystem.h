// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimeForceSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeForceChangedSignature, int32, NewTimeForce);

UCLASS()
class PROJECT_SP_API UTimeForceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Time Force")
	bool DecreaseTimeForce(int32 AmountToDecrease);

	UFUNCTION(BlueprintPure, Category = "Time Force")
	int32 GetCurrentTimeForce() const;

	UFUNCTION(BlueprintCallable, Category = "Time Force")
	void ResetTimeForce();

	UPROPERTY(BlueprintAssignable, Category = "Time Force")
	FOnTimeForceChangedSignature OnTimeForceChanged;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Time Force", meta = (AllowPrivateAccess = "true"))
	int32 CurrentTimeForce;

	int32 StartingTimeForce;

	void HandleGameOver();
};
