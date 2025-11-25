// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_MoveCharacter.generated.h"

UENUM(BlueprintType)
enum class EMoveTargetType : uint8
{
	ToTarget    UMETA(DisplayName = "타겟에게 이동"),
	ToHome      UMETA(DisplayName = "원래 위치로 복귀"),
    ToCurrentLocationWithOffset UMETA(DisplayName = "현재 위치 기준 이동")
};

UCLASS()
class PROJECT_SP_API UTask_MoveCharacter : public UCombatTask
{
	GENERATED_BODY()

public:
    // UCombatTask의 가상 함수들을 오버라이드합니다.
    virtual bool IsLatent() const override { return true; }
    virtual void ExecuteTask_Implementation() override;
    virtual void TickTask(float DeltaTime) override;

protected:
    /** 이동 타입을 결정합니다. */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    EMoveTargetType MoveType = EMoveTargetType::ToTarget;

    /** 이동 속도 (초당 유닛) */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    float MoveSpeed = 2000.0f;

    /** 타겟에게 이동 시, 이 거리만큼 앞에서 멈춥니다. */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    FVector TargetOffset_3D = FVector::ZeroVector;
    /** 회전 속도 (RInterpTo의 속도값) */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    float RotationSpeed = 10.0f;

    // 이동 시 재생할 몽타주 (점프, 대쉬, 백스텝 등)
    UPROPERTY(EditAnywhere, Category = "Task Properties|Visuals")
    TObjectPtr<UAnimMontage> MovementMontage;

    // True: 적을 바라보며 이동 (백스텝, 공중 사격)
    // False: 이동하는 방향을 바라봄 (대쉬, 달리기)
    UPROPERTY(EditAnywhere, Category = "Task Properties|Visuals")
    bool bFaceTargetWhileMoving = false;

    // True: 지형 무시하고 공중으로 직선 이동 (점프, 공중 대쉬용)
    UPROPERTY(EditAnywhere, Category = "Task Properties|Visuals")
    bool bIgnoreGround = false;

    // [추가] 이동 중 카메라 연출을 사용할지 여부
    UPROPERTY(EditAnywhere, Category = "Task Properties|Camera")
    bool bUseActionCamera = false;

    // [추가] 캐릭터 기준 카메라 오프셋 (예: X=-300, Z=150 -> 등 뒤 위쪽)
    UPROPERTY(EditAnywhere, Category = "Task Properties|Camera")
    FVector ActionCameraOffset = FVector(-300.0f, 0.0f, 150.0f);

    // [추가] 캐릭터 기준 카메라 회전 오프셋 (예: Pitch=-15 -> 아래를 내려다봄)
    // 캐릭터가 바라보는 방향(Rotation)에 이 값을 더해서 최종 각도를 결정합니다.
    UPROPERTY(EditAnywhere, Category = "Task Properties|Camera")
    FRotator ActionCameraRotationOffset = FRotator(-15.0f, 0.0f, 0.0f);

    // [추가] 카메라 이동 속도 (부드럽게 따라가기 위함)
    UPROPERTY(EditAnywhere, Category = "Task Properties|Camera")
    float ActionCameraSmoothSpeed = 10.0f;
    //카메라의 시야 각(줌 인, 줌 아웃)
    UPROPERTY(EditAnywhere, Category = "Task Properties|Camera")
    float ActionCameraFOV = 90.0f;

    // 원래 카메라 위치 복구용
    bool bWasActionCameraUsed = false;

private:
    /** 이동해야 할 최종 목적지 위치 */
    FVector TargetLocation;
    /** 도달해야 할 최종 회전값 */
    FRotator TargetRotation;

    /** 이동이 완료되었는지 확인하기 위한 플래그 */
    bool bIsMoveComplete = false;
	
    //캐릭터의 최대 이동 속도
    float OriginalMaxWalkSpeed = 0.0f;

    uint8 OriginalMovementMode;
};
