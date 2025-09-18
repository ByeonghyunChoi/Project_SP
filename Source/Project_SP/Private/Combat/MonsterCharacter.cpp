#include "Combat/MonsterCharacter.h"
#include "Combat/ActionComponent.h"
#include "Event/GameEventComponent.h"
#include "Combat/MonsterAIController.h" 
#include "Animation/AnimMontage.h"


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
    AIControllerClass = AMonsterAIController::StaticClass();
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    SetFaction(EFaction::Enemy);

    if (ActionComponent)
    {
        ActionComponent->InitializeDefaultActions(DefaultActionIDs);
    }
}




UMonsterGroupObject* AMonsterCharacter::GetCombatMonsterGroup() const
{
    return CombatMonsterGroup;
}

void AMonsterCharacter::SetWeaknessType(EDamageType NewType)
{
    WeaknessType = NewType;
}

void AMonsterCharacter::OnTurnBegin()
{
    if (GetCombatPawnState() == ECombatPawnState::Defeated) return;

    UE_LOG(LogTemp, Log, TEXT("Monster '%s' Turn Began."), *GetName());
    SetCombatPawnState(ECombatPawnState::PerformingAction);

    // 자신을 조종하는 AI 컨트롤러를 찾아 턴이 시작되었음을 알립니다.
    AMonsterAIController* AICon = GetController<AMonsterAIController>();
    if (AICon)
    {
        AICon->OnTurnBegan();
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