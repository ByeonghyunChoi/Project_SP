#include "Component/GameEventComponent.h"
#include "Character/CombatPawn.h" 
#include "Data/ActionData.h" 



UGameEventComponent::UGameEventComponent() { PrimaryComponentTick.bCanEverTick = false; }
void UGameEventComponent::BeginPlay() { Super::BeginPlay(); }

void UGameEventComponent::BroadcastDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, ACombatPawn* InstigatorPawn, UDamageType* DamageType) { OnDamageReceived.Broadcast(DamagedPawn, DamageAmount, InstigatorPawn, DamageType); }
void UGameEventComponent::BroadcastTurnStarted(ACombatPawn* TurnPawn) { OnTurnStarted.Broadcast(TurnPawn); }
void UGameEventComponent::BroadcastTurnEnded(ACombatPawn* TurnPawn) { OnTurnEnded.Broadcast(TurnPawn); }
void UGameEventComponent::BroadcastActionPerformed(ACombatPawn* PerformingPawn, FActionData PerformedActionData) { OnActionPerformed.Broadcast(PerformingPawn, PerformedActionData); }
void UGameEventComponent::BroadcastActionExecutionFinished(ACombatPawn* FinishedPawn) { OnActionExecutionFinished.Broadcast(FinishedPawn); }
void UGameEventComponent::BroadcastCombatPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState) { OnCombatPawnStateChanged.Broadcast(Pawn, NewState); }
void UGameEventComponent::BroadcastTargetListChanged(const TArray<ACombatPawn*>& NewTargets){ OnTargetListChanged.Broadcast(NewTargets); }
void UGameEventComponent::BroadcastParryAttempted(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult) { OnParryAttempted.Broadcast(ParriedAttacker, ParryingPlayer, ParryResult); }
void UGameEventComponent::BroadcastInterruptRequest(ACombatPawn* Instigator){ OnInterruptRequest.Broadcast(Instigator); }
void UGameEventComponent::BroadcastParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration){ OnParryWindowOpened.Broadcast(Attacker, AttackType, Duration);}
void UGameEventComponent::BroadcastParryWindowClosed(ACombatPawn* Attacker){ OnParryWindowClosed.Broadcast(Attacker);}