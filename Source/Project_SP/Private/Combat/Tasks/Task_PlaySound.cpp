// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_PlaySound.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Character/CombatPawn.h"

void UTask_PlaySound::ExecuteTask_Implementation()
{
    if (SoundToPlay && GetWorld())
    {
        if (bPlayAs2D)
        {
            // UI 사운드처럼 2D로 재생합니다.
            UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
        }
        else if (Instigator)
        {
            // 가해자(Instigator)의 위치에서 3D 사운드를 재생합니다.
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, Instigator->GetActorLocation());
        }
    }
    // 사운드 재생은 "즉시" 시작되는 작업이므로, 바로 태스크를 종료하고 다음으로 넘어갑니다.
    FinishTask();
}
