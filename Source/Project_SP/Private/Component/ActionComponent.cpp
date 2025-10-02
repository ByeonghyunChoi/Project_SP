// Component/ActionComponent.cpp

#include "Component/ActionComponent.h"
#include "Combat/GameAction.h"
#include "Data/ActionData.h"
#include "Character/CombatPawn.h"

UActionComponent::UActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

	for (const UGameAction* Action : GrantedActions)
	{
		if (Action && Action->GetActionID() == ActionID)
		{
			return; // 이미 있으면 추가하지 않음
		}
	}

	const FActionData* FoundRow = ActionDataTable->FindRow<FActionData>(ActionID, TEXT(""));
	if (FoundRow && FoundRow->GameActionClass)
	{
		UGameAction* NewAction = NewObject<UGameAction>(GetOwner(), FoundRow->GameActionClass);
		if (NewAction)
		{
			NewAction->Initialize(this, ActionID);
			GrantedActions.Add(NewAction);
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
			return;
		}
	}
}

bool UActionComponent::StartActionByID(ACombatPawn* Instigator, FName ActionID, const TArray<ACombatPawn*>& Targets)
{
	for (UGameAction* Action : GrantedActions)
	{
		if (Action && Action->GetActionID() == ActionID)
		{
			if (Action->CanStartAction(Instigator))
			{
				Action->StartAction(Instigator, Targets);
				return true;
			}
			return false; // 조건이 맞지 않아 실행 실패
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("ActionID '%s'를 찾을 수 없습니다."), *ActionID.ToString());
	return false; // 해당 ID의 액션을 찾지 못함
}

bool UActionComponent::GetActionData(FName ActionID, FActionData& OutActionData) const
{
	if (!ActionDataTable)
	{
		return false; 
	}

	const FActionData* FoundRow = ActionDataTable->FindRow<FActionData>(ActionID, TEXT(""));
	if (FoundRow)
	{
		OutActionData = *FoundRow;
		return true;
	}

	return false; 
}