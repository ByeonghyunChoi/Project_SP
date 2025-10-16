#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SignalTask.generated.h"

UCLASS()
class PROJECT_SP_API UAnimNotify_SignalTask : public UAnimNotify
{
	GENERATED_BODY()

protected:
	// 이 노티파이가 보낼 신호의 이름입니다.
	// 에디터의 애니메이션 몽타주에서 직접 이 이름을 설정할 수 있습니다.
	UPROPERTY(EditAnywhere, Category = "AnimNotify")
	FName SignalName;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};