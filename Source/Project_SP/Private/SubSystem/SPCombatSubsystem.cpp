// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPCombatSubsystem.h"

void USPCombatSubsystem::SetPendingEncounter(const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage)
{
	if (EncounterData)
	{
		CurrentEncounter = EncounterData;
		CurrentAdvantage = Advantage;
		ResetTutorialStep();

		UE_LOG(LogTemp, Log, TEXT("[CombatSubsystem] 전투 데이터 저장 완료! 맵: %s, 상태: %d"),
			*EncounterData->CombatLevelName.ToString(), (int32)Advantage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatSubsystem] 전투 데이터가 비어있습니다!"));
	}
}

ETutorialStage USPCombatSubsystem::GetCurrentTutorialStage() const
{
	if (CurrentEncounter && CurrentEncounter->bIsTutorial)
	{
		return CurrentEncounter->TutorialStage;
	}
	return ETutorialStage::None;
}

void USPCombatSubsystem::AdvanceTutorialStep()
{
	CurrentTutorialStep++;
	UE_LOG(LogTemp, Warning, TEXT("[튜토리얼 각본] 유저가 미션을 완수했습니다! Step %d(으)로 넘어갑니다."), CurrentTutorialStep);
}

void USPCombatSubsystem::ResetTutorialStep()
{
	CurrentTutorialStep = 1;
	UE_LOG(LogTemp, Log, TEXT("[튜토리얼 각본] 튜토리얼 단계가 1로 초기화되었습니다."));
}
