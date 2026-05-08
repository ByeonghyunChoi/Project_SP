// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SPBattleDirector.h"
#include "Game/SPBattleCameraActor.h"
#include "Component/SPStatusEffectComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

ASPBattleDirector::ASPBattleDirector()
{

}

void ASPBattleDirector::InitDirector(ASPBattleCameraActor* InCameraManager)
{
	if (InCameraManager)
	{
		CameraManager = InCameraManager;
		UE_LOG(LogTemp, Log, TEXT("[BattleDirector] 감독 초기화 완료! 카메라 매니저 연동 성공."));
	}
}

void ASPBattleDirector::RegisterMonster(AActor* MonsterActor)
{
	if (!MonsterActor) return;

	// 몬스터의 상태이상 컴포넌트를 찾아서 내 안테나(HandleStatusVisual)를 꽂습니다.
	if (USPStatusEffectComponent* StatusComp = MonsterActor->FindComponentByClass<USPStatusEffectComponent>())
	{
		// 안전장치: 혹시나 중복 바인딩 되는 것을 막기 위해 한 번 지우고 다시 연결합니다.
		StatusComp->OnStatusVisualTriggered.RemoveDynamic(this, &ASPBattleDirector::HandleStatusVisual);
		StatusComp->OnStatusVisualTriggered.AddDynamic(this, &ASPBattleDirector::HandleStatusVisual);
	}
}

void ASPBattleDirector::FinishStatusSequence()
{
	bIsPlayingVisual = false;

	// 대기열에 다음 연출이 있고, 그게 방금 연출한 애랑 '같은 몬스터'라면?
	bool bNextIsSameActor = (VisualQueue.Num() > 0 && VisualQueue[0].TargetActor == CurrentPlayingActor);

	if (bNextIsSameActor)
	{
		// 족쇄 풀지 말고, 카메라도 움직이지 말고 바로 다음 연출(독 -> 화상 등) 재생!
		TryPlayNextVisual();
	}
	else
	{
		// 🌟 [핵심] 이 몬스터의 모든 연출이 끝났습니다! AI 족쇄를 여기서 확실하게 풉니다!
		if (CurrentPlayingActor)
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentPlayingActor);
			if (ASC)
			{
				if (!ASC->HasMatchingGameplayTag(FSPGameplayTags::Get().State_Death))
				{
					ASC->SetLooseGameplayTagCount(FSPGameplayTags::Get().State_Status_VisualPlaying, 0);
					UE_LOG(LogTemp, Warning, TEXT("[%s] AI 족쇄 해제."), *CurrentPlayingActor->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s] 타겟이 사망하여 AI 족쇄를 유지합니다."), *CurrentPlayingActor->GetName());
				}
			}
		}

		// 만약 카메라가 이 녀석을 찍고 있었다면 줌아웃을 지시합니다.
		if (CurrentFocusedActor && CurrentFocusedActor == CurrentPlayingActor)
		{
			OnTargetFocusEnd(CurrentFocusedActor);
			CurrentFocusedActor = nullptr;
		}

		CurrentPlayingActor = nullptr; // 초기화

		// 대기열에 다른 몬스터의 연출이 남아있다면 이어서 진행
		if (VisualQueue.Num() > 0)
		{
			TryPlayNextVisual();
		}
	}
}

void ASPBattleDirector::HandleStatusVisual(AActor* TargetActor, FGameplayTag StatusTag, bool bIsInstant)
{
	if (bIsInstant)
	{
		PlayInstantStatusSequence(TargetActor, StatusTag);
		return; // 여기서 끝내버려서 큐에 안 들어가게 막습니다.
	}

	// 🌟 2. 도트 데미지(턴 시작)일 때만 대기열(Queue)에 줄을 세웁니다.
	FStatusVisualRequest NewRequest;
	NewRequest.TargetActor = TargetActor;
	NewRequest.StatusTag = StatusTag;
	NewRequest.bIsInstant = false;
	VisualQueue.Add(NewRequest);

	TryPlayNextVisual();
}

void ASPBattleDirector::TryPlayNextVisual()
{
	if (bIsPlayingVisual || VisualQueue.Num() == 0) return;

	bIsPlayingVisual = true;

	FStatusVisualRequest Request = VisualQueue[0];
	VisualQueue.RemoveAt(0);

	// 🌟 1. 현재 연출 중인 타겟을 기록합니다! (줌인 여부 상관없음)
	CurrentPlayingActor = Request.TargetActor;

	// 2. 도트 딜(턴 시작)일 때만 카메라를 줌인합니다.
	if (!Request.bIsInstant)
	{
		if (CurrentFocusedActor != Request.TargetActor)
		{
			CurrentFocusedActor = Request.TargetActor;
			OnTargetFocusBegin(CurrentFocusedActor); // 블루프린트 카메라 줌인 지시
		}
	}

	// 3. 이펙트 + 데미지 연출 시작 지시! (블루프린트로 토스)
	PlayStatusSequence(Request.TargetActor, Request.StatusTag, Request.bIsInstant);
}

