#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ActionData.h" // FActionData를 위해 포함
#include "Delegates/DelegateCombinations.h" // 델리게이트 매크로를 위해 포함
#include "GameEventComponent.generated.h"

// ACombatPawn과 ECombatPawnState에 대한 포워드 선언
class ACombatPawn;
enum class ECombatPawnState : uint8; // UENUM은 포워드 선언 가능 (자세한 내용은 CombatPawn.h에 정의된 ECombatPawnState UENUM 아래에)


// --- 델리게이트 선언들 ---
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageReceived, class ACombatPawn*, DamagedPawn, float, DamageAmount, ACombatPawn*, InstigatorPawn, class UDamageType*, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, class ACombatPawn*, CombatPawn, float, CurrentHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStarted, class ACombatPawn*, TurnPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnded, class ACombatPawn*, TurnPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionPerformed, class ACombatPawn*, PerformingPawn, FActionData, PerformedActionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionExecutionFinished, class ACombatPawn*, FinishedPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatPawnStateChanged, class ACombatPawn*, Pawn, ECombatPawnState, NewState); // 새로운 델리게이트 추가


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
    FOnDamageReceived OnDamageReceived;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnHealthChanged OnHealthChanged;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnTurnStarted OnTurnStarted;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnTurnEnded OnTurnEnded;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnActionPerformed OnActionPerformed;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnActionExecutionFinished OnActionExecutionFinished;
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnCombatPawnStateChanged OnCombatPawnStateChanged; // 새로운 델리게이트 인스턴스

    // --- 브로드캐스트 함수들 ---
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastDamageReceived(ACombatPawn* DamagedPawn, float DamageAmount, ACombatPawn* InstigatorPawn, UDamageType* DamageType);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastHealthChanged(ACombatPawn* CombatPawn, float NewHealth);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastTurnStarted(ACombatPawn* TurnPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastTurnEnded(ACombatPawn* TurnPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastActionPerformed(ACombatPawn* PerformingPawn, FActionData PerformedActionData);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastActionExecutionFinished(ACombatPawn* FinishedPawn);
    UFUNCTION(BlueprintCallable, Category = "Game Events")
    void BroadcastCombatPawnStateChanged(ACombatPawn* Pawn, ECombatPawnState NewState); // 새로운 브로드캐스트 함수

};