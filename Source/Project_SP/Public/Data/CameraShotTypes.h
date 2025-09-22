// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraShotTypes.generated.h"

// 카메라가 어떤 대상을 기준으로 위치할지 정의하는 열거형
UENUM(BlueprintType)
enum class ECameraShotTarget : uint8
{
    Attacker        UMETA(DisplayName = "공격자 기준"),
    Target          UMETA(DisplayName = "피격자 기준"),
    Midpoint        UMETA(DisplayName = "중간 지점 기준"),
    World           UMETA(DisplayName = "월드 고정 위치")
};

// 데이터 테이블의 한 행을 구성할 구조체
USTRUCT(BlueprintType)
struct FCameraShotData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    ECameraShotTarget TargetType = ECameraShotTarget::World;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot", meta = (EditCondition = "TargetType == ECameraShotTarget::World"))
    FTransform WorldTransform;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot", meta = (EditCondition = "TargetType != ECameraShotTarget::World"))
    FVector CameraOffset = FVector(-500.f, 0.f, 200.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    float InterpolationSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot")
    float FieldOfView = 90.0f;

    // 이 샷이 재생될 때 함께 재생할 카메라 쉐이크
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Effects")
    TSubclassOf<class UCameraShakeBase> CameraShake;
};

