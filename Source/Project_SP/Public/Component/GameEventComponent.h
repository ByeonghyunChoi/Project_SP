#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ActionData.h" 
#include "Delegates/DelegateCombinations.h"
#include "Combat/CombatTypes.h"
#include "GameEventComponent.generated.h"

// ACombatPawn과 ECombatPawnState에 대한 포워드 선언
class ACombatPawn;
enum class ECombatPawnState : uint8; // UENUM은 포워드 선언 가능 (자세한 내용은 CombatPawn.h에 정의된 ECombatPawnState UENUM 아래에)


// --- 델리게이트 선언들 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageFinalized, ACombatPawn*, DamagedPawn, float, DamageAmount, EDamageFloaterType, DamageType, ACombatPawn*, InstigatorPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStarted, class ACombatPawn*, TurnPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnded, class ACombatPawn*, TurnPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionPerformed, class ACombatPawn*, PerformingPawn, FActionData, PerformedActionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionExecutionFinished, class ACombatPawn*, FinishedPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatPawnStateChanged, class ACombatPawn*, Pawn, ECombatPawnState, NewState); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetListChanged, const TArray<ACombatPawn*>&, NewTargets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnParryAttempted, ACombatPawn*, ParriedAttacker, ACombatPawn*, ParryingPlayer, EParryResult, ParryResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInterruptRequest, ACombatPawn*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnParryWindowOpened, ACombatPawn*, Attacker, EDamageType, AttackType, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParryWindowClosed, ACombatPawn*, Attacker);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UGameEventComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGameEventComponent();

protected:
    virtual void BeginPlay() override;

public:
    // --- Event Dispatcher 인스턴스 ---
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnDamageFinalized OnDamageFinalized;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnTurnStarted OnTurnStarted;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnTurnEnded OnTurnEnded;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnActionPerformed OnActionPerformed;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnActionExecutionFinished OnActionExecutionFinished;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnCombatPawnStateChanged OnCombatPawnStateChanged;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnTargetListChanged OnTargetListChanged;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnParryAttempted OnParryAttempted;

    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnInterruptRequest OnInterruptRequest;

    UPROPERTY(BlueprintAssignable, Category = "Game Events | Parry")
    FOnParryWindowOpened OnParryWindowOpened;
    UPROPERTY(BlueprintAssignable, Category = "Game Events | Parry")
    FOnParryWindowClosed OnParryWindowClosed;

    // --- 브로드캐스트 함수들 ---
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastDamageFinalized(ACombatPawn* DamagedPawn, float DamageAmount, EDamageFloaterType DamageType, ACombatPawn* InstigatorPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastTurnStarted(ACombatPawn* TurnPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastTurnEnded(ACombatPawn* TurnPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastActionPerformed(ACombatPawn* PerformingPawn, FActionData PerformedActionData);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastActionExecutionFinished(ACombatPawn* FinishedPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastCombatPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastTargetListChanged(const TArray<ACombatPawn*>& NewTargets);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastParryAttempted(ACombatPawn* ParriedAttacker, ACombatPawn* ParryingPlayer, EParryResult ParryResult);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastInterruptRequest(ACombatPawn* Instigator);
    UFUNCTION(BlueprintCallable, Category = "Game Events | Parry")
    void BroadcastParryWindowOpened(ACombatPawn* Attacker, EDamageType AttackType, float Duration);
    UFUNCTION(BlueprintCallable, Category = "Game Events | Parry")
    void BroadcastParryWindowClosed(ACombatPawn* Attacker);
};