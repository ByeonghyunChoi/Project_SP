// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_MoveCharacter.h"
#include "Character/CombatPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BattleManager.h"
#include "Component/BattleTurnComponent.h"

void UTask_MoveCharacter::ExecuteTask_Implementation()
{
    bIsMoveComplete = false;
    OriginalMaxWalkSpeed = 0.0f;

    if (!Instigator)
    {
        FinishTask();
        return;
    }

    UCharacterMovementComponent* MovementComponent = Instigator->GetCharacterMovement();
    if (MovementComponent)
    {
        // 원래 속도를 저장합니다.
        OriginalMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
        // 이 태스크에 설정된 MoveSpeed 값으로 최대 속도를 변경합니다.
        MovementComponent->MaxWalkSpeed = MoveSpeed;
    }
    else
    {
        FinishTask();
    }

    if (MoveType == EMoveTargetType::ToHome)
    {
        TargetLocation = Instigator->GetHomeTransform().GetLocation(); 

        FVector AverageOpponentLocation = FVector::ZeroVector;
        int32 OpponentCount = 0;
        if (BattleManager)
        {
            const EFaction MyFaction = Instigator->GetFaction();
            for (ACombatPawn* Pawn : BattleManager->GetAllCombatants())
            {
                if (Pawn && Pawn->GetFaction() != MyFaction && Pawn->GetCombatPawnState() != ECombatPawnState::Defeated)
                {
                    AverageOpponentLocation += Pawn->GetActorLocation();
                    OpponentCount++;
                }
            }
        }

        if (OpponentCount > 0)
        {
            AverageOpponentLocation /= OpponentCount;
            TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, AverageOpponentLocation);
        }
        else
        {
            TargetRotation = Instigator->GetHomeTransform().GetRotation().Rotator();
            UE_LOG(LogTemp, Warning, TEXT("UTask_MoveCharacter (ToHome): No opponents found to look at, using Home rotation."));
        }
        UE_LOG(LogTemp, Warning, TEXT("UTask_MoveCharacter ExecuteTask: TargetLocation = %s, TargetRotation = %s"), *TargetLocation.ToString(), *TargetRotation.ToString());
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
    

    if (bIsMoveComplete || !Instigator) return;
    UCharacterMovementComponent* MovementComponent = Instigator->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("무브먼트 컴포넌트 없음"));
        FinishTask();
    }

    FVector CurrentLocation = Instigator->GetActorLocation();

    FVector DirectionToTarget = TargetLocation - CurrentLocation;
    DirectionToTarget.Normalize();
    MovementComponent->AddInputVector(DirectionToTarget * 1.0f, true);

    FRotator CurrentRotation = Instigator->GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
    Instigator->GetController()->SetControlRotation(NewRotation); 

    if (FVector::DistSquaredXY(CurrentLocation, TargetLocation) < FMath::Square(10.0f)) 
    {
        MovementComponent->StopMovementImmediately(); 
        if (OriginalMaxWalkSpeed > 0.0f)
        {
            MovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed;
        }
        bIsMoveComplete = true;
        UE_LOG(LogTemp, Warning, TEXT("UTask_MoveCharacter: 목표 도달 조건 충족! (DistXY)"));
    }

    if (bIsMoveComplete)
    {
        UE_LOG(LogTemp, Warning, TEXT("UTask_MoveCharacter: 이동 완료! FinishTask() 호출 시도."));
        FinishTask();
    }
}