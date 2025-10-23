// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CombatCameraShotDirector.generated.h"

class ACombatPawn;
class UCombatCameraComponent;

UCLASS(Blueprintable, BlueprintType)
class PROJECT_SP_API UCombatCameraShotDirector : public UObject
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, Category = "Camera Shot Director")
    FTransform CalculateCameraTransform(ACombatPawn* Attacker, ACombatPawn* Target, UCombatCameraComponent* CameraComponent);
    virtual FTransform CalculateCameraTransform_Implementation(ACombatPawn* Attacker, ACombatPawn* Target, UCombatCameraComponent* CameraComponent);

    /** 이 Director가 계산을 위해 Target 액터가 반드시 필요한지 여부 (예: 피격자 중심 샷) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shot Director")
    bool bRequiresTarget = false;
	
};
