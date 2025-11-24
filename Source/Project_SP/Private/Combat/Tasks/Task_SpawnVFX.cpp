// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_SpawnVFX.h"
#include "Character/CombatPawn.h"
#include "NiagaraFunctionLibrary.h"

void UTask_SpawnVFX::ExecuteTask_Implementation()
{
	if (VFXSystem && GetWorld())
	{
		FVector SpawnLoc = FVector::ZeroVector;
		FRotator SpawnRot = FRotator::ZeroRotator;

		// 타겟이 있으면 타겟 위치, 없으면 시전자 위치
		AActor* TargetActor = (bSpawnAtTarget && Targets.IsValidIndex(0)) ? Targets[0] : Instigator;

		if (TargetActor)
		{
			SpawnLoc = TargetActor->GetActorLocation() + LocationOffset;
			SpawnRot = TargetActor->GetActorRotation();
		}

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), VFXSystem, SpawnLoc, SpawnRot, Scale);
	}
	FinishTask();
}
