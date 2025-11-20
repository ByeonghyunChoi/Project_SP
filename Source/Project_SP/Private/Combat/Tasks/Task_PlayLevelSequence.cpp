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
	// [수정] 변수 값에 따라 카메라 컷 제어 (Disable = !Use)
	Settings.bDisableCameraCuts = !bUseCameraCuts;
	Settings.bHideHud = true;

	ALevelSequenceActor* TempActor = nullptr; // 1. 임시 포인터 생성

	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(), SequenceAsset, Settings, TempActor); // 2. 임시 포인터 전달

	SequenceActor = TempActor;

	if (SequencePlayer && SequenceActor)
	{
		// 2. 동적 바인딩
		if (Instigator)
		{
			// (옵션) 시퀀스 원점을 시전자 위치로 이동하려면 아래 주석 해제
			// SequenceActor->SetActorTransform(Instigator->GetActorTransform());
			SequenceActor->AddBindingByTag(AttackerTag, Instigator);
		}

		if (Targets.IsValidIndex(0) && Targets[0])
		{
			SequenceActor->AddBindingByTag(TargetTag, Targets[0]);
		}

		// 3. 재생
		SequencePlayer->OnFinished.AddDynamic(this, &UTask_PlayLevelSequence::OnSequenceFinished);
		SequencePlayer->Play();

		// [핵심] 대기 옵션이 꺼져있으면 즉시 태스크 종료
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
	if (bWaitForCompletion)
	{
		FinishTask();
	}
}
