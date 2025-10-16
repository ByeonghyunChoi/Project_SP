// Combat/Tasks/Task_WaitForAnimNotify.cpp

#include "Combat/Tasks/Task_WaitForAnimNotify.h"
#include "Character/CombatPawn.h"
#include "Animation/AnimInstance.h"

void UTask_WaitForAnimNotify::ExecuteTask_Implementation()
{
	if (Instigator && !NotifyName.IsNone())
	{
		UAnimInstance* AnimInstance = Instigator->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			// --- 수정된 부분: AddUObject를 사용하여 C++ 전용 델리게이트에 바인딩합니다 ---
			NotifyDelegateHandle = AnimInstance->OnMontageNotifyBegin.AddUObject(this, &UTask_WaitForAnimNotify::OnNotifyReceived);
			return; // 성공적으로 바인딩했으므로 여기서 종료
		}
	}

	// 실패 시 즉시 작업을 종료합니다.
	FinishTask();
}

void UTask_WaitForAnimNotify::OnNotifyReceived(FName ReceivedNotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (ReceivedNotifyName == NotifyName)
	{
		// 내가 기다리던 이름의 노티파이가 울렸으므로, 작업을 종료합니다.
		FinishTask();
	}
}

// BattleManager가 ClearTaskQueue를 호출하는 등, 작업이 끝나기 전에 강제로 중단될 때를 대비하여
// 델리게이트가 쓰레기로 남지 않도록 정리해주는 함수입니다.
void UTask_WaitForAnimNotify::FinishTask()
{
	if (Instigator)
	{
		UAnimInstance* AnimInstance = Instigator->GetMesh()->GetAnimInstance();
		if (AnimInstance && NotifyDelegateHandle.IsValid())
		{
			AnimInstance->OnMontageNotifyBegin.Remove(NotifyDelegateHandle);
			NotifyDelegateHandle.Reset();
		}
	}

	// 반드시 부모의 FinishTask를 호출해야 합니다.
	Super::FinishTask();
}