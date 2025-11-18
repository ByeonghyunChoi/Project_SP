// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraShotTypes.generated.h"

class UCombatCameraShotDirector;

// 데이터 테이블의 한 행을 구성할 구조체
USTRUCT(BlueprintType)
struct FCameraShotData : public FTableRowBase
{
    GENERATED_BODY()

public:
    FCameraShotData()
        : DirectorClass(nullptr)
        , InterpolationSpeed(5.0f) // 기본값 유지
        , FieldOfView(90.0f)       // 기본값 유지
        , bInstantCut(false)
        , CameraShake(nullptr)
    {
    }

    FCameraShotData(
        TSubclassOf<UCombatCameraShotDirector> InDirectorClass,
        float InInterpolationSpeed,
        float InFieldOfView,
        bool InbInstantCut,
        TSubclassOf<class UCameraShakeBase> InCameraShake
    )
        : DirectorClass(InDirectorClass)
        , InterpolationSpeed(InInterpolationSpeed)
        , FieldOfView(InFieldOfView)
        , bInstantCut(InbInstantCut)
        , CameraShake(InCameraShake)
    {
    }

    /** 이 샷의 카메라 Transform을 계산할 Director 클래스입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Logic", meta = (AllowAbstract = "false"))
    TSubclassOf<UCombatCameraShotDirector> DirectorClass;

    // --- 공통 설정 ---
    /** 보간 이동 시 속도 (bInstantCut이 false일 때 사용) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Common")
    float InterpolationSpeed = 5.0f;

    /** 목표 카메라 시야각 (Field of View) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Common")
    float FieldOfView = 90.0f;

    /** true이면 보간 없이 즉시 이 샷으로 카메라를 설정합니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Common")
    bool bInstantCut = false;

    /** 이 샷이 재생될 때 함께 재생할 카메라 쉐이크 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shot|Effects")
    TSubclassOf<class UCameraShakeBase> CameraShake;
};

