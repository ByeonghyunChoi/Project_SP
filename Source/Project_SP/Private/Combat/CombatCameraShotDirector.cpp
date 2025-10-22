// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatCameraShotDirector.h"
#include "Component/CombatCameraComponent.h"
#include "GameFramework/Actor.h"

FTransform UCombatCameraShotDirector::CalculateCameraTransform_Implementation(ACombatPawn* Attacker, ACombatPawn* Target, UCombatCameraComponent* CameraComponent)
{
    if (CameraComponent && CameraComponent->GetOwner())
    {
        return CameraComponent->GetOwner()->GetActorTransform();
    }

    return FTransform::Identity;
}
