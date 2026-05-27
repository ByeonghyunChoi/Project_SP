// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SPTutorialManagerComponent.h"
#include "SubSystem/SPCombatSubsystem.h"
#include "Character/SPGASPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

USPTutorialManagerComponent::USPTutorialManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USPTutorialManagerComponent::StartTutorialScenario()
{
	bIsTutorialActive = true;
	CurrentStep = -1;

	if (TutorialPopupClass && !ActivePopupWidget)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			ActivePopupWidget = CreateWidget<UUserWidget>(PC, TutorialPopupClass);
			if (ActivePopupWidget) ActivePopupWidget->AddToViewport(100);
		}
	}

	// 2. 🌟 핵심: 1.5초 뒤에 게임을 멈추고 튜토리얼을 본격적으로 시작합니다!
	// 이 1.5초 동안 전투 HUD가 화면에 예쁘게 나타나고 전투 돌입 연출이 나옵니다.
	GetWorld()->GetTimerManager().SetTimer(TutorialStartTimer, this, &USPTutorialManagerComponent::ExecuteTutorialPause, 3.0f, false);
}

void USPTutorialManagerComponent::EndTutorial()
{
	bIsTutorialActive = false;
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (ActivePopupWidget)
	{
		ActivePopupWidget->RemoveFromParent();
		ActivePopupWidget = nullptr;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Tutorial] 튜토리얼 종료. 자유 전투 전환."));
}

void USPTutorialManagerComponent::AdvanceStep()
{
	if (!bIsTutorialActive) return;

	CurrentStep++;
	UE_LOG(LogTemp, Warning, TEXT("[Tutorial] 각본 이동 -> Step %d"), CurrentStep);

	if (CurrentStep >= 8) EndTutorial();
	else ProcessCurrentStep();
}

void USPTutorialManagerComponent::OnPlayerTurnStarted()
{
	if (!bIsTutorialActive) return;
	// 플레이어 턴에 멈춰야 하는 스텝들 (1차전 설명, 5: 스킬, 7: 시간간섭)
	if (CurrentStep == 0 || CurrentStep == 1 || CurrentStep == 5 || CurrentStep == 7)
	{
		ProcessCurrentStep();
	}
}

void USPTutorialManagerComponent::OnParryTimingTriggered()
{
	if (!bIsTutorialActive) return;
	// 적 공격 도중 패링 타이밍 도달
	if (CurrentStep == 6) ProcessCurrentStep();
}

bool USPTutorialManagerComponent::CanProcessInput(FGameplayTag InputTag) const
{
	if (!bIsTutorialActive) return true;

	const FSPGameplayTags& Tags = FSPGameplayTags::Get();

	switch (CurrentStep)
	{
	case -1: case 0: case 1: case 2: case 3: return false; // 설명 단계: 모든 입력 차단 (UI '다음' 버튼만 가능)
	case 4: return InputTag.MatchesTagExact(Tags.Battle_Action_Attack); // 일반 공격 유도
	case 5: return InputTag.MatchesTagExact(Tags.Battle_Action_Skill);  // 무기 스킬 유도
	case 6: return InputTag.MatchesTagExact(Tags.Battle_Action_Parry);  // 패링 유도
	case 7: return InputTag.MatchesTagExact(Tags.Battle_Action_TimeInterference); // 시간 간섭 유도
	default: return false;
	}
}

void USPTutorialManagerComponent::ProcessCurrentStep()
{
	// 1. 게임 강제 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	// 2. UI에 화면 갱신 방송 (블루프린트에서 구멍 위치/텍스트 변경)
	OnTutorialStepChanged.Broadcast(CurrentStep);
}

void USPTutorialManagerComponent::ExecuteTutorialPause()
{
	CurrentStep = 0;
	ProcessCurrentStep();
}



