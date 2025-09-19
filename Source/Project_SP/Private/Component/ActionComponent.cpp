// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ActionComponent.h"
#include "Combat/GameAction.h"
#include "Data/ActionData.h"
#include "Character/CombatPawn.h"

// Sets default values for this component's properties
UActionComponent::UActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UActionComponent::InitializeDefaultActions(const TArray<FName>& DefaultActionIDs)
{
	for (const FName& ActionID : DefaultActionIDs)
	{
		GrantAction(ActionID);
	}
}

void UActionComponent::GrantAction(FName ActionID)
{
	if (!ActionDataTable) return;

	for (UGameAction* Action : GrantedActions)
	{
		if (Action && Action->GetActionID() == ActionID)
		{
			return; // 이미 있으면 추가하지 않음
		}
	}

	const FActionData* FoundRow = ActionDataTable->FindRow<FActionData>(ActionID, TEXT("GrantAction"));
	if (FoundRow && FoundRow->GameActionClass)
	{
		UGameAction* NewAction = NewObject<UGameAction>(GetOwner(), FoundRow->GameActionClass);
		if (NewAction)
		{
			NewAction->Initialize(this, ActionID);
			GrantedActions.Add(NewAction);
			OnActionListChanged.Broadcast(this); // UI 업데이트를 위해 이벤트 방송
		}
	}
}

void UActionComponent::RemoveAction(FName ActionID)
{
	for (int32 i = GrantedActions.Num() - 1; i >= 0; --i)
	{
		if (GrantedActions[i] && GrantedActions[i]->GetActionID() == ActionID)
		{
			GrantedActions.RemoveAt(i);
			OnActionListChanged.Broadcast(this); // UI 업데이트를 위해 이벤트 방송
			return;
		}
	}
}

bool UActionComponent::StartActionByID(ACombatPawn* Instigator, FName ActionID, const TArray<ACombatPawn*>& Targets)
{
	if (ActiveAction) return false;

	for (UGameAction* Action : GrantedActions)
	{
		if (Action && Action->GetActionID() == ActionID)
		{
			if (Action->CanStartAction(Instigator))
			{
				ActiveAction = Action;
				ActiveAction->StartAction(Instigator, Targets);
				return true;
			}
			return false;
		}
	}
	return false;
}

void UActionComponent::EndActiveAction(ACombatPawn* Instigator)
{
	if (ActiveAction)
	{
		UGameAction* ActionToEnd = ActiveAction;
		ActiveAction = nullptr;
		ActionToEnd->EndAction(Instigator);
	}
}



