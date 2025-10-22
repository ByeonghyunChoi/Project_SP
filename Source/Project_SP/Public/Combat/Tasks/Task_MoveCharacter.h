// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_MoveCharacter.generated.h"

UENUM(BlueprintType)
enum class EMoveTargetType : uint8
{
	ToTarget    UMETA(DisplayName = "타겟에게 이동"),
	ToHome      UMETA(DisplayName = "원래 위치로 복귀")
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
    float AttackOffset = 50.0f;

    /** 회전 속도 (RInterpTo의 속도값) */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    float RotationSpeed = 10.0f;

private:
    /** 이동해야 할 최종 목적지 위치 */
    FVector TargetLocation;
    /** 도달해야 할 최종 회전값 */
    FRotator TargetRotation;

    /** 이동이 완료되었는지 확인하기 위한 플래그 */
    bool bIsMoveComplete = false;
	
};
