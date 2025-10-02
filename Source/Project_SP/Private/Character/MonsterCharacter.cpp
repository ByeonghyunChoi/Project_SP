#include "Character/MonsterCharacter.h"
#include "Combat/MonsterAIController.h" 
#include "Component/ActionComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/WidgetComponent.h" 
#include "Combat/MonsterGroupObject.h"


AMonsterCharacter::AMonsterCharacter()
{
    AIControllerClass = AMonsterAIController::StaticClass();
}

void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    SetFaction(EFaction::Enemy);

    if (ActionComponent)
    {
        ActionComponent->InitializeDefaultActions(DefaultActionIDs);
    }
}

void AMonsterCharacter::OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets)
{
    if (GetCombatPawnState() == ECombatPawnState::Defeated) return;

    UE_LOG(LogTemp, Log, TEXT("Monster '%s' Turn Began."), *GetCharacterDisplayName().ToString());
    SetCombatPawnState(ECombatPawnState::PerformingAction);

    AMonsterAIController* AICon = GetController<AMonsterAIController>();
    if (AICon)
    {
        AICon->OnTurnBegan(PotentialTargets);
    }
}

void AMonsterCharacter::PlayActionMontage(FName ActionID)
{
    const TSoftObjectPtr<UAnimMontage>* FoundMontage = ActionMontageMap.Find(ActionID);
    if (FoundMontage)
    {
        if (UAnimMontage* MontageToPlay = FoundMontage->LoadSynchronous())
        {
            PlayAnimMontage(MontageToPlay);
        }
    }
}