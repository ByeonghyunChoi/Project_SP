// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SPBattleCameraActor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASPBattleCameraActor::ASPBattleCameraActor()
{
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComp);

	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	CameraSpringArm->SetupAttachment(RootComponent);

	CameraSpringArm->TargetArmLength = 400.0f;
	CameraSpringArm->SetRelativeRotation(FRotator(-15.0f, 180.0f, 0.0f)); // 약간 위에서 정면을 내려다보는 각도
	CameraSpringArm->bDoCollisionTest = false; // 충돌로 인한 카메라 튕김 방지

	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);

}

void ASPBattleCameraActor::InitCameraManager(APlayerController* PC)
{
	if (PC)
	{
		CachedPC = PC;
		// 현재 PC가 보고 있는 화면(보통 플레이어 캐릭터)을 '돌아갈 고향'으로 저장해둡니다.
		DefaultViewTarget = PC->GetViewTarget();
		UE_LOG(LogTemp, Log, TEXT("[CameraManager] 초기화 완료. 복귀 타겟: %s"), *DefaultViewTarget->GetName());
	}
}

void ASPBattleCameraActor::FocusOnTarget(AActor* TargetActor, float BlendTime)
{
	if (!TargetActor || !CachedPC) return;

	// 몬스터의 현재 위치와 방향을 가져옵니다.
	FVector TargetLoc = TargetActor->GetActorLocation();
	FVector TargetForward = TargetActor->GetActorForwardVector();

	// 🌟 몬스터의 정면에서 약간 떨어져서 몬스터를 바라보게 설정!
	// (예: 몬스터 정면 방향으로 100만큼 앞, 높이는 50 위로)
	FVector CameraPos = TargetLoc + (TargetForward * 100.0f) + FVector(0, 0, 50.0f);
	SetActorLocation(CameraPos);

	// 몬스터를 바라보게 회전
	FRotator LookAtRot = (TargetLoc - CameraPos).Rotation();
	SetActorRotation(LookAtRot);

	// 즉시 화면 전환! (BlendTime이 0.0f일 경우)
	CachedPC->SetViewTargetWithBlend(this, BlendTime);
}

void ASPBattleCameraActor::ReturnToMainView(float BlendTime)
{
	if (!CachedPC || !DefaultViewTarget) return;

	// 🌟 3. 아까 저장해둔 고향(플레이어 뷰)으로 다시 부드럽게 날아갑니다.
	CachedPC->SetViewTargetWithBlend(DefaultViewTarget, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic);

	UE_LOG(LogTemp, Log, TEXT("[CameraManager] 메인 뷰로 복귀!"));
}



