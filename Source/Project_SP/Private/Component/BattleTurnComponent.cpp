#include "Component/BattleTurnComponent.h"
#include "Component/AttributesComponent.h"
#include "EngineUtils.h"


// Sets default values for this component's properties
UBattleTurnComponent::UBattleTurnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UBattleTurnComponent::BeginPlay()
{
	Super::BeginPlay();

	AttributesComp = GetOwner()->FindComponentByClass<UAttributesComponent>();
}

void UBattleTurnComponent::StartTurn()
{
    bIsMyTurn = true;
    ActionValue = 0.f; // 턴을 가졌으므로 행동 게이지를 0으로 초기화
}

void UBattleTurnComponent::EndTurn()
{
    bIsMyTurn = false;
}

void UBattleTurnComponent::AdvanceActionValue(float DeltaTime)
{
    if (bIsMyTurn || !AttributesComp) return;

    // 자신의 속도에 비례하여 행동 게이지 증가
    const float Speed = AttributesComp->GetCurrentStats().fMovementSpeed;
    ActionValue += Speed * DeltaTime;
}

float UBattleTurnComponent::GetActionValue()
{
    return ActionValue;
}

float UBattleTurnComponent::GetActionThreshold()
{
    return ActionThreshold;
}

bool UBattleTurnComponent::IsReadyForTurn() const
{
    return ActionValue >= ActionThreshold;
}
