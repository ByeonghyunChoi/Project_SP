// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_MoveCharacter.h"
#include "Character/CombatPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BattleManager.h"
#include "Component/BattleTurnComponent.h"
#include "Component/CombatCameraComponent.h"

void UTask_MoveCharacter::ExecuteTask_Implementation()
{
	bIsMoveComplete = false;

	if (!Instigator) { FinishTask(); return; }

	UCharacterMovementComponent* MoveComp = Instigator->GetCharacterMovement();
	if (MoveComp)
	{
		OriginalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		OriginalMovementMode = (uint8)MoveComp->MovementMode; // 원래 모드 저장

		
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->MaxWalkSpeed = MoveSpeed;
	}

	// 1. 몽타주 재생
	if (MovementMontage)
	{
		Instigator->PlayAnimMontage(MovementMontage);
	}

	// 2. 목표 위치 계산
	FVector CurrentLoc = Instigator->GetActorLocation();
	FVector TargetBaseLoc = CurrentLoc; // 기본값

	if (MoveType == EMoveTargetType::ToTarget && Targets.IsValidIndex(0))
	{
		TargetBaseLoc = Targets[0]->GetActorLocation();

		// 타겟을 바라보는 방향 기준으로 오프셋 적용
		FVector DirToTarget = (TargetBaseLoc - CurrentLoc).GetSafeNormal2D();
		FRotator LookAtRot = DirToTarget.Rotation();

		// TargetOffset_3D.X가 양수면 "타겟 앞", Z가 양수면 "타겟 위"
		// X축(앞뒤) 계산: 타겟 위치에서 내 쪽으로(반대 방향) Offset.X 만큼 뺌
		FVector FinalOffset = (-DirToTarget * TargetOffset_3D.X) + (FVector::UpVector * TargetOffset_3D.Z);
		TargetLocation = TargetBaseLoc + FinalOffset;

		TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, TargetBaseLoc);
	}
	else if (MoveType == EMoveTargetType::ToHome)
	{
		TargetBaseLoc = Instigator->GetHomeTransform().GetLocation();
		TargetLocation = TargetBaseLoc; // Home은 오프셋 없이 정확히 복귀
		TargetRotation = Instigator->GetHomeTransform().GetRotation().Rotator();
	}
	else if (MoveType == EMoveTargetType::ToCurrentLocationWithOffset)
	{
		// 현재 위치 기준 (제자리 점프 등)
		// Actor의 현재 회전 기준으로 오프셋 적용 (로컬 좌표)
		FVector LocalOffset = Instigator->GetActorRotation().RotateVector(TargetOffset_3D);
		TargetLocation = CurrentLoc + LocalOffset;

		// 회전은 유지하거나 타겟을 봄
		if (Targets.IsValidIndex(0))
		{
			TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, Targets[0]->GetActorLocation());
		}
		else
		{
			TargetRotation = Instigator->GetActorRotation();
		}
	}

	if (bUseActionCamera && BattleManager)
	{
		if (UCombatCameraComponent* CamComp = BattleManager->GetCameraComponent())
		{
			// 이동 시작 시점에는 카메라를 '캐릭터 등 뒤'로 부드럽게 이동 시작
			// 여기서 바로 확 튀면 어색하므로 Tick에서 부드럽게 처리합니다.
			bWasActionCameraUsed = true;
		}
	}
}

void UTask_MoveCharacter::TickTask(float DeltaTime)
{
    if (bIsMoveComplete || !Instigator) return;

    FVector CurrentLoc = Instigator->GetActorLocation();

    // 1. [이동 로직]
    FVector NewLoc = FMath::VInterpConstantTo(CurrentLoc, TargetLocation, DeltaTime, MoveSpeed);

    if (!bIgnoreGround)
    {
        NewLoc.Z = CurrentLoc.Z; // "땅 파고 들어가지 마!"
    }

    Instigator->SetActorLocation(NewLoc, false);

    // 2. [회전 로직]
    FRotator CurrentRot = Instigator->GetActorRotation();
    FRotator GoalRot = TargetRotation;

    if (bFaceTargetWhileMoving && Targets.IsValidIndex(0))
    {
        GoalRot = UKismetMathLibrary::FindLookAtRotation(CurrentLoc, Targets[0]->GetActorLocation());
    }
    else if (!bFaceTargetWhileMoving)
    {
        FVector MoveDir = (TargetLocation - CurrentLoc).GetSafeNormal();
        if (!MoveDir.IsNearlyZero())
        {
            GoalRot = MoveDir.Rotation();
        }
    }
    GoalRot.Pitch = 0.f;
    GoalRot.Roll = 0.f;

    FRotator NewRot = FMath::RInterpTo(CurrentRot, GoalRot, DeltaTime, RotationSpeed);
    Instigator->SetActorRotation(NewRot);


    // 3. [액션 카메라 로직]
    if (bUseActionCamera && BattleManager)
    {
        if (UCombatCameraComponent* CamComp = BattleManager->GetCameraComponent())
        {
            FVector CharLoc = Instigator->GetActorLocation();
            FRotator CharRot = Instigator->GetActorRotation();

            FVector CamGoalLoc = CharLoc + CharRot.RotateVector(ActionCameraOffset);
            FRotator CamGoalRot = CharRot + ActionCameraRotationOffset;

            CamComp->SetCameraTargetLocation(CamGoalLoc, CamGoalRot, ActionCameraFOV, ActionCameraSmoothSpeed);
        }
    }

    // 4. [도착 판정] (여기가 핵심 수정!)
    float DistanceSquared = 0.0f;

    if (bIgnoreGround)
    {
        // 공중 이동(점프)은 3D 거리 전체를 체크 (높이까지 맞아야 함)
        DistanceSquared = FVector::DistSquared(CurrentLoc, TargetLocation);
    }
    else
    {
        // [해결책] 지상 이동(복귀)은 높이(Z)를 무시하고 XY 평면 거리만 체크!
        // 이렇게 하면 바닥 높이가 조금 달라도 도착으로 인정됨
        DistanceSquared = FVector::DistSquaredXY(CurrentLoc, TargetLocation);
    }

    // 50.0f (50cm) 이내면 도착으로 간주
    if (DistanceSquared < FMath::Square(50.0f))
    {
        // 이동 완료!
        bIsMoveComplete = true;

        // [옵션] 깔끔한 마무리를 위해 위치 강제 동기화 
        // (단, 지상 이동일 때는 Z축은 건드리지 않고 XY만 맞춤)
        if (!bIgnoreGround)
        {
            FVector FinalLoc = FVector(TargetLocation.X, TargetLocation.Y, CurrentLoc.Z);
            Instigator->SetActorLocation(FinalLoc);
        }
        else
        {
            Instigator->SetActorLocation(TargetLocation);
        }

        UCharacterMovementComponent* MoveComp = Instigator->GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->StopMovementImmediately();
            MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed;

            if (bIgnoreGround)
            {
                MoveComp->SetMovementMode(EMovementMode(OriginalMovementMode));
            }
        }

        if (MovementMontage)
        {
            Instigator->StopAnimMontage(MovementMontage);
        }

        FinishTask();
        return;
    }
}