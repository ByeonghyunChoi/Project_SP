// Combat/Tasks/Task_WaitForAnimNotify.h

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_WaitForAnimNotify.generated.h"

UCLASS()
class PROJECT_SP_API UTask_WaitForAnimNotify : public UCombatTask
{
	GENERATED_BODY()

public:
	virtual void ExecuteTask_Implementation() override;

	// BattleManager가 직접 호출할 수 있도록 public으로 변경합니다.
	void OnNotifyReceived(FName ReceivedNotifyName);

protected:
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	FName NotifyName;
};