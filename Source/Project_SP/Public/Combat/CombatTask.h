// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CombatTask.generated.h"

class ABattleManager;
class ACombatPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTaskFinished);

UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced)
class PROJECT_SP_API UCombatTask : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FOnTaskFinished OnTaskFinished;

    UFUNCTION(BlueprintNativeEvent, Category = "Combat Task")
    void ExecuteTask();
    virtual void ExecuteTask_Implementation();

    virtual void Initialize(ABattleManager* InBattleManager, ACombatPawn* InInstigator, const TArray<ACombatPawn*>& InTargets);

    virtual bool IsLatent() const { return false; }

    virtual void TickTask(float DeltaTime) {}

    virtual void OnNotifyReceived(FName NotifyName) {}
    
    ACombatPawn* GetInstigator() const { return Instigator; }

    UPROPERTY(EditAnywhere, Category = "Combat Task")
    bool bApplyToTarget = false;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Combat Task")
    TObjectPtr<ABattleManager> BattleManager;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Task")
    TObjectPtr<ACombatPawn> Instigator;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Task")
    TArray<TObjectPtr<ACombatPawn>> Targets;

    UFUNCTION(BlueprintCallable, Category = "Combat Task")
    virtual void FinishTask();
};
