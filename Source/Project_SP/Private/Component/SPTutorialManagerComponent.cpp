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
	CurrentStep = 0;

	if (TutorialPopupClass && !ActivePopupWidget)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			ActivePopupWidget = CreateWidget<UUserWidget>(PC, TutorialPopupClass);
			if (ActivePopupWidget) ActivePopupWidget->AddToViewport(100);
		}
	}
	ProcessCurrentStep();
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

	// 🌟 [각본 매핑] 무기 스킬(Step 6)과 시간 간섭(Step 7)은 플레이어 턴이 시작될 때 멈춥니다!
	if (CurrentStep == 6 || CurrentStep == 7)
	{
		FTimerHandle TurnStartDelayTimer;
		GetWorld()->GetTimerManager().SetTimer(
			TurnStartDelayTimer,
			this,
			&USPTutorialManagerComponent::ProcessCurrentStep, // 1.5초 뒤에 실행할 함수
			1.0f,
			false
		);
	}
}

void USPTutorialManagerComponent::OnParryTimingTriggered()
{
	if (!bIsTutorialActive) return;

	// 🌟 [각본 매핑] 적이 나를 치려는 패링 타이밍(Step 5)이 오면 세상을 얼립니다!
	if (CurrentStep == 5)
	{
		ProcessCurrentStep();
	}
}

bool USPTutorialManagerComponent::CanProcessInput(FGameplayTag InputTag) const
{
	if (!bIsTutorialActive) return true;

	const FSPGameplayTags& Tags = FSPGameplayTags::Get();

	if (CurrentStep >= 5 && InputTag.MatchesTagExact(Tags.Battle_Action_Parry))
	{
		return true;
	}

	switch (CurrentStep)
	{
	case 0: case 1: case 2: case 3: case 5: return false; // 설명 단계: 클릭(Next)만 허용
	case 4: return InputTag.MatchesTagExact(Tags.Battle_Action_Attack); // 🌟 일반 공격 유도
	case 6: return InputTag.MatchesTagExact(Tags.Battle_Action_Skill);  // 🌟 무기 스킬 유도
	case 7: return InputTag.MatchesTagExact(Tags.Battle_Action_TimeInterference); // 🌟 시간 간섭 유도
	default: return false;
	}
}

void USPTutorialManagerComponent::HideTutorialPopup()
{
	if (ActivePopupWidget)
	{
		ActivePopupWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USPTutorialManagerComponent::ProcessCurrentStep()
{
	// 1. UI 방송을 먼저 해서 위젯들이 생성되고 배치될 시간을 줍니다.
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	if (ActivePopupWidget)
	{
		ActivePopupWidget->SetVisibility(ESlateVisibility::Visible);
	}

	OnTutorialStepChanged.Broadcast(CurrentStep);
}