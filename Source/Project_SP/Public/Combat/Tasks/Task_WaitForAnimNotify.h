// Combat/Tasks/Task_WaitForAnimNotify.h

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_WaitForAnimNotify.generated.h"

UCLASS()
class PROJECT_SP_API UTask_WaitForAnimNotify : public UCombatTask
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Task Properties")
	FName NotifyName;

public:
	virtual void ExecuteTask_Implementation() override;
	virtual void FinishTask() override; // 작업이 강제로 중단될 때를 대비한 정리 함수

private:
	UFUNCTION()
	void OnNotifyReceived(FName ReceivedNotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	// 델리게이트 연결을 관리하기 위한 핸들입니다.
	FDelegateHandle NotifyDelegateHandle;
};