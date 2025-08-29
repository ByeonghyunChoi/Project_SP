#include "Event/GameEventComponent.h"
#include "Combat/CombatPawn.h" 
#include "Data/ActionData.h" 



UGameEventComponent::UGameEventComponent() { PrimaryComponentTick.bCanEverTick = false; }
void UGameEventComponent::BeginPlay() { Super::BeginPlay(); }

void UGameEventComponent::BroadcastDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, ACombatPawn* InstigatorPawn, UDamageType* DamageType) { OnDamageReceived.Broadcast(DamagedPawn, DamageAmount, InstigatorPawn, DamageType); }
void UGameEventComponent::BroadcastHealthChanged(ACombatPawn* CombatPawn, float NewHealth) { OnHealthChanged.Broadcast(CombatPawn, NewHealth); }
void UGameEventComponent::BroadcastTurnStarted(ACombatPawn* TurnPawn) { OnTurnStarted.Broadcast(TurnPawn); }
void UGameEventComponent::BroadcastTurnEnded(ACombatPawn* TurnPawn) { OnTurnEnded.Broadcast(TurnPawn); }
void UGameEventComponent::BroadcastActionPerformed(ACombatPawn* PerformingPawn, FActionData PerformedActionData) { OnActionPerformed.Broadcast(PerformingPawn, PerformedActionData); }
void UGameEventComponent::BroadcastActionExecutionFinished(ACombatPawn* FinishedPawn) { OnActionExecutionFinished.Broadcast(FinishedPawn); }
void UGameEventComponent::BroadcastCombatPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState) { OnCombatPawnStateChanged.Broadcast(Pawn, NewState); }
void UGameEventComponent::BroadcastTargetListChanged(const TArray<ACombatPawn*>& NewTargets){ OnTargetListChanged.Broadcast(NewTargets); }
void UGameEventComponent::BroadcastParryWindowChanged(ACombatPawn* Attacker, EDamageType AttackType, bool bIsWindowOpen) { OnParryWindowChanged.Broadcast(Attacker, AttackType, bIsWindowOpen); }
void UGameEventComponent::BroadcastParryAttempted(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult) { OnParryAttempted.Broadcast(ParriedAttacker, ParryingPlayer, ParryResult); }