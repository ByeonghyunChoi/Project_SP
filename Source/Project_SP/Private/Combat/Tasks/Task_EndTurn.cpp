// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_EndTurn.h"
#include "Character/CombatPawn.h"
#include "Component/ActionComponent.h"

void UTask_EndTurn::ExecuteTask_Implementation()
{
	if (Instigator && Instigator->GetActionComponent())
	{
		Instigator->GetActionComponent()->EndActiveAction(Instigator);
	}
	FinishTask();
}

