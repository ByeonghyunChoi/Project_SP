// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CameraShotTypes.h"
#include "CombatCameraComponent.generated.h"

class ACineCameraActor;
class UDataTable;
class UCombatCameraShotDirector;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UCombatCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatCameraComponent();

    void InitializeCamera(FName CameraTag);

    void PlayDefaultShot(AActor* Attacker, AActor* Target);
    void PlayAttackerShot(AActor* Attacker, AActor* Target);
    void PlayEnemyShot(AActor* Attacker, AActor* Target);
    void PlayParryShot(AActor* Parrier, AActor* Attacker);

    void PlayShot(FName ShotName, AActor* Attacker = nullptr, AActor* Target = nullptr);

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditDefaultsOnly, Category = "Camera Director")
    TObjectPtr<UDataTable> ShotDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "Config|Shot Names")
    FName DefaultWideShotName = TEXT("Shot_DefaultWide");

    UPROPERTY(EditDefaultsOnly, Category = "Config|Shot Names")
    FName PlayerAttackShotName = TEXT("Shot_PlayerAttack");

    UPROPERTY(EditDefaultsOnly, Category = "Config|Shot Names")
    FName EnemyAttackShotName = TEXT("Shot_EnemyAttack");

    UPROPERTY(EditDefaultsOnly, Category = "Config|Shot Names")
    FName ParryImpactShotName = TEXT("Shot_ParryImpact");

private:
    UPROPERTY()
    TObjectPtr<ACineCameraActor> ControlledCamera;

    FVector TargetLocation;
    FRotator TargetRotation;
    float TargetFieldOfView;
    float CurrentInterpolationSpeed;
		
};
