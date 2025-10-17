// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CombatCameraComponent.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UCombatCameraComponent::UCombatCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}


void UCombatCameraComponent::InitializeCamera(FName CameraTag)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), CameraTag, FoundActors);
    if (FoundActors.Num() > 0)
    {
        ControlledCamera = Cast<ACineCameraActor>(FoundActors[0]);
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && ControlledCamera)
        {
            PC->SetViewTargetWithBlend(ControlledCamera, 0.0f);
        }
    }
}

void UCombatCameraComponent::PlayDefaultShot(AActor* Attacker, AActor* Target)
{
    PlayShot(TEXT("Shot_DefaultWide"), Attacker, Target);
}

void UCombatCameraComponent::PlayAttackerShot(AActor* Attacker, AActor* Target)
{
    PlayShot(TEXT("Shot_PlayerAttack"), Attacker, Target);
}

void UCombatCameraComponent::PlayEnemyShot(AActor* Attacker, AActor* Target)
{
    PlayShot(TEXT("Shot_EnemyAttack"), Attacker, Target);
}

void UCombatCameraComponent::PlayParryShot(AActor* Parrier, AActor* Attacker)
{
    PlayShot(TEXT("Shot_ParryImpact"), Parrier, Attacker);
}

void UCombatCameraComponent::PlayShot(FName ShotName, AActor* Attacker, AActor* Target)
{
    if (!ShotDataTable || !ControlledCamera) return;
    FCameraShotData* ShotData = ShotDataTable->FindRow<FCameraShotData>(ShotName, TEXT(""));
    if (!ShotData) return;

    if (ShotData->CameraShake)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC) PC->ClientStartCameraShake(ShotData->CameraShake);
    }

    // 목표 위치/회전 계산
    FVector BaseLocation = FVector::ZeroVector;
    AActor* LookAtTarget = nullptr;

    switch (ShotData->TargetType)
    {
    case ECameraShotTarget::Attacker:
        if (Attacker) { BaseLocation = Attacker->GetActorLocation(); LookAtTarget = Attacker; }
        break;
    case ECameraShotTarget::Target:
        if (Target) { BaseLocation = Target->GetActorLocation(); LookAtTarget = Target; }
        break;
    case ECameraShotTarget::Midpoint:
        if (Attacker && Target) {
            BaseLocation = (Attacker->GetActorLocation() + Target->GetActorLocation()) / 2.0f;
            LookAtTarget = nullptr; // 중간 지점을 보도록
        }
        break;
    case ECameraShotTarget::World:
        TargetLocation = ShotData->WorldTransform.GetLocation();
        TargetRotation = ShotData->WorldTransform.GetRotation().Rotator();
        break;
    }

    if (ShotData->TargetType != ECameraShotTarget::World)
    {
        TargetLocation = BaseLocation + ShotData->CameraOffset;
        TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, LookAtTarget ? LookAtTarget->GetActorLocation() : BaseLocation);
    }

    TargetFieldOfView = ShotData->FieldOfView;
    CurrentInterpolationSpeed = ShotData->InterpolationSpeed;
    SetComponentTickEnabled(true);
}

void UCombatCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ControlledCamera)
    {
        FVector CurrentLocation = ControlledCamera->GetActorLocation();
        FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, CurrentInterpolationSpeed);
        ControlledCamera->SetActorLocation(NewLocation);

        FRotator CurrentRotation = ControlledCamera->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, CurrentInterpolationSpeed);
        ControlledCamera->SetActorRotation(NewRotation);

        UCineCameraComponent* CineComponent = ControlledCamera->GetCineCameraComponent();
        float CurrentFOV = CineComponent->FieldOfView;
        float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFieldOfView, DeltaTime, CurrentInterpolationSpeed);
        CineComponent->SetFieldOfView(NewFOV);

        if (CurrentLocation.Equals(TargetLocation, 1.0f))
        {
            SetComponentTickEnabled(false);
        }
    }
}

