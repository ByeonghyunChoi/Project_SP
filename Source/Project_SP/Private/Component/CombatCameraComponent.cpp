// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CombatCameraComponent.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Data/CameraShotTypes.h"
#include "Combat/CombatCameraShotDirector.h"
#include "Character/CombatPawn.h"
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
    PlayShot(DefaultWideShotName, Attacker, Target);
}

void UCombatCameraComponent::PlayAttackerShot(AActor* Attacker, AActor* Target)
{
    PlayShot(PlayerAttackShotName, Attacker, Target);
}

void UCombatCameraComponent::PlayEnemyShot(AActor* Attacker, AActor* Target)
{
    PlayShot(EnemyAttackShotName, Attacker, Target);
}

void UCombatCameraComponent::PlayParryShot(AActor* Parrier, AActor* Attacker)
{
    PlayShot(ParryImpactShotName, Parrier, Attacker);
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
    FVector FinalTargetLocation = FVector::ZeroVector;
    FRotator FinalTargetRotation = FRotator::ZeroRotator;

    if (!Attacker)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayShot '%s': Attacker is required but missing."), *ShotName.ToString());
        return; // Attacker 없이는 샷 실행 불가
    }

    FVector BaseLocation = Attacker->GetActorLocation();

    AActor* LookAtActor = Target ? Target : Attacker;
    FinalTargetRotation = UKismetMathLibrary::FindLookAtRotation(FinalTargetLocation, LookAtActor->GetActorLocation());

    if (ShotData->bInstantCut)
    {
        SetComponentTickEnabled(false);
        ControlledCamera->SetActorLocationAndRotation(FinalTargetLocation, FinalTargetRotation);
        if (UCineCameraComponent* CineComponent = ControlledCamera->GetCineCameraComponent())
        {
            CineComponent->SetFieldOfView(ShotData->FieldOfView);
        }
    }
    else
    {
        TargetLocation = FinalTargetLocation; 
        TargetRotation = FinalTargetRotation; 
        TargetFieldOfView = ShotData->FieldOfView;
        CurrentInterpolationSpeed = ShotData->InterpolationSpeed;
        SetComponentTickEnabled(true);
    }
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

