// Combat/Tasks/Task_PlayAnimation.h

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_PlayAnimation.generated.h"

class UAnimMontage;

UCLASS()
class PROJECT_SP_API UTask_PlayAnimation : public UCombatTask
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	TObjectPtr<UAnimMontage> MontageToPlay;

public:
	virtual void ExecuteTask_Implementation() override;
};