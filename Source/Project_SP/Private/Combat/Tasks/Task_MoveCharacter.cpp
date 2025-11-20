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

		// [핵심] 공중 이동이면 'Flying' 모드로 전환 (중력 무시)
		if (bIgnoreGround)
		{
			MoveComp->SetMovementMode(MOVE_Flying);
		}
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
	Instigator->SetActorLocation(NewLoc, true);

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


	// 3. [액션 카메라 로직] (매 프레임 실행)
	if (bUseActionCamera && BattleManager)
	{
		if (UCombatCameraComponent* CamComp = BattleManager->GetCameraComponent())
		{
			FVector CharLoc = Instigator->GetActorLocation();
			FRotator CharRot = Instigator->GetActorRotation();

			// 캐릭터 등 뒤 오프셋 계산
			FVector CamGoalLoc = CharLoc + CharRot.RotateVector(ActionCameraOffset);
			FRotator CamGoalRot = CharRot;
			CamGoalRot.Pitch -= 10.0f;

			// 카메라 이동 명령
			// (SetCameraTargetLocation 함수가 없다면 구현 필요)
			CamComp->SetCameraTargetLocation(CamGoalLoc, CamGoalRot, ActionCameraSmoothSpeed);
		}
	}


	// 4. [도착 판정]
	if (FVector::DistSquared(CurrentLoc, TargetLocation) < FMath::Square(10.0f))
	{
		// 이동 완료!
		bIsMoveComplete = true;

		UCharacterMovementComponent* MoveComp = Instigator->GetCharacterMovement();
		if (MoveComp)
		{
			MoveComp->StopMovementImmediately();
			MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed;

			// 원래 이동 모드 복구
			if (bIgnoreGround)
			{
				MoveComp->SetMovementMode(EMovementMode(OriginalMovementMode));
			}
		}

		// 몽타주 정지
		if (MovementMontage)
		{
			Instigator->StopAnimMontage(MovementMontage);
		}

		// 태스크 종료
		FinishTask();
	}
}