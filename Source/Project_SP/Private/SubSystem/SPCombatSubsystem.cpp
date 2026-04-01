// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPCombatSubsystem.h"

void USPCombatSubsystem::SetPendingEncounter(const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage)
{
	if (EncounterData)
	{
		CurrentEncounter = EncounterData;
		CurrentAdvantage = Advantage;

		UE_LOG(LogTemp, Log, TEXT("[CombatSubsystem] 전투 데이터 저장 완료! 맵: %s, 상태: %d"),
			*EncounterData->CombatLevelName.ToString(), (int32)Advantage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatSubsystem] 전투 데이터가 비어있습니다!"));
	}
}
