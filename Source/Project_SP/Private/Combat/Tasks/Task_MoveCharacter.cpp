// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_MoveCharacter.h"
#include "Character/CombatPawn.h"
#include "Kismet/KismetMathLibrary.h"

void UTask_MoveCharacter::ExecuteTask_Implementation()
{
    bIsMoveComplete = false;

    if (!Instigator)
    {
        FinishTask();
        return;
    }

    if (MoveType == EMoveTargetType::ToHome)
    {
        // "집으로 복귀"를 선택한 경우
        TargetLocation = Instigator->GetHomeTransform().GetLocation();
        TargetRotation = Instigator->GetHomeTransform().GetRotation().Rotator();
    }
    else // (MoveType == EMoveTargetType::ToTarget)
    {
        // "타겟에게 이동"을 선택한 경우
        if (Targets.Num() == 0 || !Targets[0])
        {
            FinishTask(); // 타겟이 없으면 즉시 종료
            return;
        }

        ACombatPawn* TargetPawn = Targets[0];
        FVector TargetPawnLocation = TargetPawn->GetActorLocation();
        FVector InstigatorLocation = Instigator->GetActorLocation();

        FVector DirectionToTarget = (TargetPawnLocation - InstigatorLocation);
        DirectionToTarget.Z = 0;
        DirectionToTarget.Normalize();

        TargetLocation = TargetPawnLocation - (DirectionToTarget * AttackOffset);
        TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, TargetPawnLocation);
    }

    // ExecuteTask는 목적지만 설정할 뿐, FinishTask()를 호출하지 않습니다.
    // 실제 이동은 TickTask()에서 처리됩니다.
}

void UTask_MoveCharacter::TickTask(float DeltaTime)
{
    // 이미 완료되었거나 시전자가 없으면 더 이상 처리하지 않음
    if (bIsMoveComplete || !Instigator)
    {
        return;
    }

    FVector CurrentLocation = Instigator->GetActorLocation();

    // 1. 목적지까지 남은 거리 계산
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    // 2. 이번 프레임에 이동할 최대 거리
    float MoveDistance = MoveSpeed * DeltaTime;

    FVector NewLocation;

    if (DistanceToTarget <= MoveDistance)
    {
        // 3a. 남은 거리가 이동 거리보다 짧으면, 목적지에 정확히 도착시킴
        NewLocation = TargetLocation;
        bIsMoveComplete = true; // 이동 완료!
    }
    else
    {
        // 3b. 아직 멀었으면, 목적지 방향으로 이동 (일정한 속도)
        NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
    }

    // 4. 회전값은 부드럽게 보간
    FRotator NewRotation = FMath::RInterpTo(Instigator->GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);

    // 5. 캐릭터의 위치와 회전을 업데이트
    Instigator->SetActorLocationAndRotation(NewLocation, NewRotation);

    // 6. 이동이 완료되었으면 BattleManager에게 태스크 종료를 알림
    if (bIsMoveComplete)
    {
        FinishTask();
    }
}