// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_PlayLevelSequence.h"
#include "Character/CombatPawn.h"
#include "MovieSceneSequencePlaybackSettings.h"

void UTask_PlayLevelSequence::ExecuteTask_Implementation()
{
    if (!SequenceAsset || !GetWorld())
    {
        FinishTask();
        return;
    }

    FMovieSceneSequencePlaybackSettings Settings;
    Settings.bAutoPlay = false;
    Settings.bDisableCameraCuts = false; // 시퀀스 카메라 사용
    Settings.bHideHud = true;            // 연출 중 HUD 숨김

    ALevelSequenceActor* OutActor = nullptr;

    // 1. 시퀀스 플레이어 및 액터 생성
    SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
        GetWorld(), SequenceAsset, Settings, OutActor);

    if (SequencePlayer && OutActor)
    {
        // 2. 동적 바인딩: 태그를 이용해 실제 액터 연결
        if (Instigator)
        {
            OutActor->AddBindingByTag(AttackerTag, Instigator);
        }

        if (Targets.IsValidIndex(0) && Targets[0])
        {
            OutActor->AddBindingByTag(TargetTag, Targets[0]);
        }

        // 3. 재생
        SequencePlayer->OnFinished.AddDynamic(this, &UTask_PlayLevelSequence::OnSequenceFinished);
        SequencePlayer->Play();

        // [핵심] 대기 옵션이 꺼져있으면 즉시 태스크 종료 (다음 태스크 실행)
        if (!bWaitForCompletion)
        {
            FinishTask();
        }
    }
    else
    {
        FinishTask();
    }
}

void UTask_PlayLevelSequence::OnSequenceFinished()
{
    // bWaitForCompletion이 True였을 경우 여기서 태스크 종료
    if (bWaitForCompletion)
    {
        FinishTask();
    }
}
