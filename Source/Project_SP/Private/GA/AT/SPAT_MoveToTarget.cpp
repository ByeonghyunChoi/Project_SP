// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/AT/SPAT_MoveToTarget.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"

USPAT_MoveToTarget::USPAT_MoveToTarget(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer)
{
	bTickingTask = true; 
	bIsMoving = false;
	ElapsedTime = 0.0f;
}

USPAT_MoveToTarget* USPAT_MoveToTarget::MoveAndMontage(UGameplayAbility* OwningAbility, FName TaskInstanceName, AActor* TargetActor, FTransform TargetTransform, float DistanceOffset, float MoveDuration, UAnimMontage* MontageToPlay)
{
	USPAT_MoveToTarget* MyObj = NewAbilityTask<USPAT_MoveToTarget>(OwningAbility, TaskInstanceName);
	MyObj->TargetActor = TargetActor;
	MyObj->TargetTransform = TargetTransform;
	MyObj->DistanceOffset = DistanceOffset;
	MyObj->MoveDuration = FMath::Max(0.01f, MoveDuration); // 0으로 나누기 방지
	MyObj->MontageToPlay = MontageToPlay;
	return MyObj;
}

void USPAT_MoveToTarget::Activate()
{
	Super::Activate();

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		EndTask();
		return;
	}

	// 1. 현재 내 위치 기억
	StartLocation = Character->GetActorLocation();
	StartRotation = Character->GetActorRotation();

	// 2. 수학 계산 (어디로 갈지)
	CalculateDestination();

	// 3. 몽타주 재생 (GAS 시스템 활용)
	if (MontageToPlay && AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, 1.0f);
	}

	// 4. 이동 시작!
	bIsMoving = true;
}

void USPAT_MoveToTarget::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsMoving)
	{
		ElapsedTime += DeltaTime;

		// 0.0 에서 1.0 사이의 진행도 (예: 0.1초 지났으면 0.5)
		float Alpha = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

		// 부드러운 감속 이동 (목적지에 다다를수록 스르륵 멈춤)
		float SmoothAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);

		FVector NewLoc = FMath::Lerp(StartLocation, DestLocation, SmoothAlpha);
		FRotator NewRot = FMath::Lerp(StartRotation, DestRotation, SmoothAlpha);

		// 캐릭터 뼈대 강제 이동!
		if (ACharacter* Character = Cast<ACharacter>(GetAvatarActor()))
		{
			Character->SetActorLocationAndRotation(NewLoc, NewRot, true);
		}

		// 목적지 도착!
		if (Alpha >= 1.0f)
		{
			bIsMoving = false;

			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnCompleted.Broadcast(); // 블루프린트의 Completed 핀으로 신호 쏘기!
			}
			EndTask(); // 태스크 스스로 소멸
		}
	}
}

void USPAT_MoveToTarget::CalculateDestination()
{
	// 타겟 액터가 입력되었다면? -> "적 코앞 계산 공식" 발동!
	if (TargetActor)
	{
		FVector A = TargetActor->GetActorLocation();
		FVector B = StartLocation;

		FVector Direction = (B - A).GetSafeNormal();
		DestLocation = A + (Direction * DistanceOffset); // 적 위치에서 내 쪽으로 150만큼 뺀 위치!

		DestRotation = (A - DestLocation).Rotation();
		DestRotation.Pitch = 0.0f; // 땅에 똑바로 서 있도록 보정
		DestRotation.Roll = 0.0f;
	}
	// 타겟 액터가 없다면? -> "입력받은 좌표로 그냥 가기 (제자리 복귀용)" 발동!
	else
	{
		DestLocation = TargetTransform.GetLocation();
		DestRotation = TargetTransform.Rotator();
	}
}
