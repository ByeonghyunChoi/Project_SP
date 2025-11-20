// Component/ActionComponent.cpp

#include "Component/ActionComponent.h"
#include "Combat/GameAction.h"
#include "Data/ActionData.h"
#include "Component/AttributesComponent.h"
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
	if (ActiveAction) return false;

	for (UGameAction* Action : GrantedActions)
	{
		if (Action && Action->GetActionID() == ActionID)
		{
			if (Action->CanStartAction(Instigator))
			{
				const int32 CostSP = Action->GetData().CostSP;
				// 
				if (CostSP != 0 && Instigator && Instigator->GetAttributesComponent())
				{
					Instigator->GetAttributesComponent()->ApplySPChange(-CostSP);
				}
				// 액션이 시작되면 ActiveAction에 기록합니다.
				ActiveAction = Action;
				Action->StartAction(Instigator, Targets);

				if (Action->GetData().CooldownTurns > 0)
				{
					// 쿨타임 맵에 추가 (이미 있으면 덮어씌움)
					CooldownMap.Add(ActionID, Action->GetData().CooldownTurns);
				}
				return true;
			}
			return false; // 조건이 맞지 않아 실행 실패
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("ActionID '%s'를 찾을 수 없습니다."), *ActionID.ToString());
	return false;
}

void UActionComponent::EndActiveAction(ACombatPawn* Instigator)
{
	if (ActiveAction)
	{
		// 임시 변수에 저장해두고 ActiveAction을 먼저 null로 만들어 중복 호출을 방지합니다.
		UGameAction* ActionToEnd = ActiveAction;
		ActiveAction = nullptr;

		// 실제 액션 종료 로직(이벤트 방송)을 호출합니다.
		ActionToEnd->EndAction(Instigator);
	}
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

void UActionComponent::ResetActiveAction()
{
	ActiveAction = nullptr;
}

void UActionComponent::ReduceCooldowns()
{
	for (auto It = CooldownMap.CreateIterator(); It; ++It)
	{
		It.Value()--; // 1턴 감소
		if (It.Value() <= 0)
		{
			It.RemoveCurrent(); // 0이 되면 목록에서 제거 (사용 가능)
		}
	}
}
