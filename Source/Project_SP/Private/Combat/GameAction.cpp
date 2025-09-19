#include "Combat/GameAction.h"
#include "Component/ActionComponent.h"
#include "Component/AttributesComponent.h"
#include "Character/CombatPawn.h"
#include "Component/GameEventComponent.h"

void UGameAction::Initialize(UActionComponent* InOwningComponent, FName InActionID)
{
    OwningComponent = InOwningComponent;
    ActionID = InActionID;

    // 데이터 테이블에서 ActionID에 해당하는 데이터를 찾아 'Data' 변수에 저장
    if (OwningComponent && OwningComponent->GetActionDataTable())
    {
        const FActionData* FoundRow = OwningComponent->GetActionDataTable()->FindRow<FActionData>(ActionID, TEXT(""));
        if (FoundRow)
        {
            Data = *FoundRow;
        }
    }
}

bool UGameAction::CanStartAction_Implementation(ACombatPawn* Instigator)
{
    if (!Instigator) return false;

    // 비용이 0이면 항상 실행 가능
    if (Data.CostSP <= 0)
    {
        return true;
    }

    // 시전자의 AttributesComponent를 가져와 SP가 충분한지 확인
    UAttributesComponent* AttributesComp = Instigator->GetAttributesComponent();
    if (AttributesComp && AttributesComp->GetSkillPoint() >= Data.CostSP)
    {
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("SP가 부족하여 '%s'을(를) 사용할 수 없습니다."), *Data.DisplayName.ToString());
    return false;
}

void UGameAction::StartAction_Implementation(ACombatPawn* Instigator, const TArray<ACombatPawn*>& Targets)
{
    UE_LOG(LogTemp, Log, TEXT("'%s' 액션 시작. 시전자: %s"), *Data.DisplayName.ToString(), *Instigator->GetName());

    if (Data.CostSP > 0)
    {
        UAttributesComponent* AttributesComp = Instigator->GetAttributesComponent();
        if (AttributesComp)
        {
            AttributesComp->ApplySPChange(-Data.CostSP);
        }
    }

    EndAction(Instigator);
}

void UGameAction::EndAction(ACombatPawn* Instigator)
{
    if (Instigator && Instigator->GetGameEventComponent())
    {
        // GameEventComponent를 통해 "액션 실행이 끝났다"고 방송함.
        Instigator->GetGameEventComponent()->BroadcastActionExecutionFinished(Instigator);
    }
}
