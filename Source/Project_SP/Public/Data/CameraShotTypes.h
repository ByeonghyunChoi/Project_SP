// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraShotTypes.generated.h"

// 데이터 테이블의 한 행을 구성할 구조체
USTRUCT(BlueprintType)
struct FCameraShotData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    FVector CameraOffset = FVector(-500.f, 0.f, 200.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    float InterpolationSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    float FieldOfView = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    bool bInstantCut = false;

    // 이 샷이 재생될 때 함께 재생할 카메라 쉐이크
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Effects")
    TSubclassOf<class UCameraShakeBase> CameraShake;
};

