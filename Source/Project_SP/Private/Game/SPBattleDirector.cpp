// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SPBattleDirector.h"
#include "Game/SPBattleCameraActor.h"
#include "Component/SPStatusEffectComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Game/ASPCombatGameMode.h"

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

	bool bNextIsSameActor = (VisualQueue.Num() > 0 && VisualQueue[0].TargetActor == CurrentPlayingActor);

	if (bNextIsSameActor)
	{
		TryPlayNextVisual();
	}
	else
	{
		// 🌟 1. 턴을 넘겨줄 죽은 몬스터를 잠시 기억해둘 변수
		AActor* DeadActorToPassTurn = nullptr;

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

					// 🌟 2. 여기서 당장 EndTurn을 부르지 않고, 녀석을 기억만 해둡니다!
					DeadActorToPassTurn = CurrentPlayingActor;
				}
			}
		}

		// 카메라 줌아웃 지시
		if (CurrentFocusedActor && CurrentFocusedActor == CurrentPlayingActor)
		{
			OnTargetFocusEnd(CurrentFocusedActor);
			CurrentFocusedActor = nullptr;
		}

		// 🌟 3. 배틀 디렉터의 메모리(상태)를 완벽하게 초기화합니다.
		CurrentPlayingActor = nullptr;

		// 대기열에 다른 연출이 남아있다면 이어서 진행
		if (VisualQueue.Num() > 0)
		{
			TryPlayNextVisual();
		}

		// 🌟 4. [핵심] 내 집 청소가 모두 끝난 가장 마지막에, 비로소 다음 턴을 시작하라고 통보합니다!
		if (DeadActorToPassTurn)
		{
			if (AASPCombatGameMode* GM = Cast<AASPCombatGameMode>(GetWorld()->GetAuthGameMode()))
			{
				if (GM->GetCurrentEnemies().Num() > 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("연출 완전 종료. 게임 모드에 턴 종료를 요청합니다."));
					GM->EndTurn(DeadActorToPassTurn);
				}
			}
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

