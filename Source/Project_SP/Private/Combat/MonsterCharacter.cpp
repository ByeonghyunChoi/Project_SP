#include "Combat/MonsterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BattleManager.h"
#include "Combat/GameAction.h"
#include "Event/GameEventComponent.h"
#include "Combat/CombatStatics.h"
#include "Data/ActionData.h"


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{

}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();
    SetFaction(EFaction::Enemy);

    if (GameEventComponent)
    {
        GameEventComponent->OnTurnStarted.AddDynamic(this, &AMonsterCharacter::HandleThisMonsterTurnStarted);
    }
}



void AMonsterCharacter::HandleThisMonsterTurnStarted(ACombatPawn* TurnPawn)
{
    if (TurnPawn == this)
    {

    }
}


UMonsterGroupObject* AMonsterCharacter::GetCombatMonsterGroup() const
{
    return CombatMonsterGroup;
}

void AMonsterCharacter::OnTurnBegin()
{
}
