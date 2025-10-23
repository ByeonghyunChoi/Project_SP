// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CombatCameraComponent.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Data/CameraShotTypes.h"
#include "Combat/CombatCameraShotDirector.h"
#include "Character/CombatPawn.h"

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

    const FCameraShotData* ShotData = ShotDataTable->FindRow<FCameraShotData>(ShotName, TEXT(""));
    if (!ShotData || !ShotData->DirectorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayShot '%s': ShotData not found or DirectorClass is not set."), *ShotName.ToString());
        return;
    }

    // --- 카메라 쉐이크 ---
    if (ShotData->CameraShake)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC) PC->ClientStartCameraShake(ShotData->CameraShake);
    }

    // --- Transform 계산 ---
    UCombatCameraShotDirector* Director = NewObject<UCombatCameraShotDirector>(this, ShotData->DirectorClass);
    if (!Director) return;

    ACombatPawn* AttackerPawn = Cast<ACombatPawn>(Attacker);
    ACombatPawn* TargetPawn = Cast<ACombatPawn>(Target);

    if (Director->bRequiresTarget && !TargetPawn) // Target 유효성 검사 추가
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayShot '%s': Director requires a Target, but none provided or invalid."), *ShotName.ToString());
        return;
    }
    // Attacker는 항상 필요하다고 가정 (필요시 bRequiresAttacker 추가 가능)
    if (!AttackerPawn) {
        UE_LOG(LogTemp, Warning, TEXT("PlayShot '%s': Attacker is required but missing or invalid."), *ShotName.ToString());
        return;
    }


    FTransform FinalTargetTransform = Director->CalculateCameraTransform(AttackerPawn, TargetPawn, this);
    FVector FinalTargetLocation = FinalTargetTransform.GetLocation();
    FRotator FinalTargetRotation = FinalTargetTransform.GetRotation().Rotator();

    // --- 카메라 이동 (Instant Cut 또는 보간) ---
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

        if (UCineCameraComponent* CineComponent = ControlledCamera->GetCineCameraComponent())
        {
            float CurrentFOV = CineComponent->FieldOfView;
            float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFieldOfView, DeltaTime, CurrentInterpolationSpeed);
            CineComponent->SetFieldOfView(NewFOV);
        }

        // 목적지 도달 시 틱 비활성화 (약간의 허용 오차 포함)
        if (CurrentLocation.Equals(TargetLocation, 1.0f) && CurrentRotation.Equals(TargetRotation, 1.0f))
        {
            SetComponentTickEnabled(false);
        }
    }
}

