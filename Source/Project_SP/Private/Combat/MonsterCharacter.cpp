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

void AMonsterCharacter::DecideAction()
{
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager)
    {
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }

    FName ActionIDToPerform = NAME_None;
    if (MyActionIDs.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, MyActionIDs.Num() - 1);
        ActionIDToPerform = MyActionIDs[RandomIndex];
    }
    else
    {
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
        return;
    }
    SelectAction(ActionIDToPerform);
}

void AMonsterCharacter::SelectAction(FName ActionID)
{
    if (GetCombatPawnState() == ECombatPawnState::PerformingAction || GetCombatPawnState() == ECombatPawnState::Defeated) return;
    SelectedActionID = ActionID;

    // [추가] 공격할 타겟을 먼저 찾습니다.
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (BattleManager)
    {
        for (ACombatPawn* Combatant : BattleManager->GetAllCombatants())
        {
            if (Combatant && Combatant->GetFaction() == EFaction::Player && Combatant->GetStatsComponent() && Combatant->GetStatsComponent()->GetCurrentHealth() > 0)
            {
                CurrentTarget = Combatant;
                break;
            }
        }
    }
    if (!CurrentTarget.IsValid())
    {
        if (GameEventComponent)
        {
            GameEventComponent->BroadcastActionExecutionFinished(this);
        }
        return;
    }

    const TSoftObjectPtr<UAnimMontage>* FoundMontage = ActionMontageMap.Find(ActionID);
    if (FoundMontage && !FoundMontage->IsNull())
    {
        if (UAnimMontage* MontageToPlay = FoundMontage->LoadSynchronous())
        {
            PlayAnimMontage(MontageToPlay);
        }
    }
    else
    {
        if (GameEventComponent) GameEventComponent->BroadcastActionExecutionFinished(this);
    }
}

void AMonsterCharacter::HandleThisMonsterTurnStarted(ACombatPawn* TurnPawn)
{
    if (TurnPawn == this)
    {
        // 몬스터 AI가 행동을 결정하고 수행하도록 지시
        DecideAction();
    }
}

void AMonsterCharacter::ResolveAction()
{
    ABattleManager* BattleManager = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass()));
    if (!BattleManager || BattleManager->GetCurrentTurnCharacter() != this) return;

    EParryResult ParryResult = BattleManager->GetCurrentTurnParryResult();

    if (ParryResult != EParryResult::Success)
    {
        // [수정] 타겟을 찾는 루프 대신, 저장된 CurrentTarget을 사용합니다.
        if (CurrentTarget.IsValid())
        {
            ACombatPawn* PlayerTarget = CurrentTarget.Get();

            float FullDamage = UCombatStatics::CalculateDamage(
                GetStatsComponent(),
                PlayerTarget->GetStatsComponent(),
                GetActionDataByID(SelectedActionID).SkillCoefficient
            );

            if (ParryResult == EParryResult::PartialSuccess)
            {
                UGameplayStatics::ApplyDamage(PlayerTarget, FullDamage * 0.7f, GetController(), this, UDamageType::StaticClass());
            }
            else
            {
                UGameplayStatics::ApplyDamage(PlayerTarget, FullDamage, GetController(), this, UDamageType::StaticClass());
            }
        }
    }

    if (GameEventComponent)
    {
        GameEventComponent->BroadcastActionExecutionFinished(this);
    }
}

UMonsterGroupObject* AMonsterCharacter::GetCombatMonsterGroup() const
{
    return CombatMonsterGroup;
}