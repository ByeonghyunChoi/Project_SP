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
	CurrentStep = 1;
	ProcessStep();
}

void USPTutorialManagerComponent::OnPlayerTurnStarted()
{
}

void USPTutorialManagerComponent::OnParryTimingTriggered()
{
}

void USPTutorialManagerComponent::AdvanceStep()
{
	CurrentStep++;
	UE_LOG(LogTemp, Warning, TEXT("[Tutorial] Step %d 로 이동"), CurrentStep);

	// 각본의 끝(예: 8단계 이후)에 도달하면 튜토리얼 종료
	if (CurrentStep >= 8)
	{
		bIsTutorialActive = false;
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		if (ActivePopupWidget) ActivePopupWidget->RemoveFromParent();
	}
	else
	{
		ProcessStep();
	}
}

bool USPTutorialManagerComponent::CanProcessInput(FGameplayTag InputTag) const
{
	if (!bIsTutorialActive) return true;

	// 각본 단계별로 허용되는 태그 매칭 로직
	// Step 4면 일반 공격(Battle_Action_Attack)만 허용, 이런 식으로 구현
	return false;
}

void USPTutorialManagerComponent::ProcessStep()
{
	// 여기서 게임 일시정지, 위젯 띄우기, 하이라이트 위치 변경을 수행합니다.
	// WBP_TutorialPopup의 UpdateTutorialView 함수를 여기서 호출하세요!
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 팝업 생성/업데이트 로직...
}

