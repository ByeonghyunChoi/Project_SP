// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPCombatSubsystem.h"

void USPCombatSubsystem::SetPendingEncounter(const UCombatEncounterData* EncounterData, ECombatAdvantage Advantage)
{
	if (EncounterData)
	{
		CurrentEncounter = EncounterData;
		CurrentAdvantage = Advantage;

		UE_LOG(LogTemp, Log, TEXT("전투 준비 완료! 맵: %s, 유리함: %d"),
			*EncounterData->CombatLevelName.ToString(), (int32)Advantage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("전투 데이터가 비어있습니다!"));
	}
}
