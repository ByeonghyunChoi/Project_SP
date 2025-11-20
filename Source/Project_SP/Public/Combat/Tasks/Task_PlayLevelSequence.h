// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Task_PlayLevelSequence.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_PlayLevelSequence : public UCombatTask
{
	GENERATED_BODY()

public:
    virtual void ExecuteTask_Implementation() override;

    // [핵심] 대기 옵션이 켜져 있을 때만 Latent(대기) 태스크로 동작
    virtual bool IsLatent() const override { return bWaitForCompletion; }

protected:
    // 재생할 시퀀스 에셋
    UPROPERTY(EditAnywhere, Category = "Sequence")
    TObjectPtr<ULevelSequence> SequenceAsset;

    // 시퀀스 내에서 '공격자'로 바인딩할 트랙 태그
    UPROPERTY(EditAnywhere, Category = "Sequence")
    FName AttackerTag = TEXT("Attacker");

    // 시퀀스 내에서 '타겟'으로 바인딩할 트랙 태그
    UPROPERTY(EditAnywhere, Category = "Sequence")
    FName TargetTag = TEXT("Target");

    // True: 시퀀스가 끝날 때까지 기다림 / False: 재생만 시키고 즉시 다음 태스크 실행
    UPROPERTY(EditAnywhere, Category = "Sequence")
    bool bWaitForCompletion = true;

    UPROPERTY(EditAnywhere, Category = "Sequence")
    bool bUseCameraCuts = true;

private:
    UPROPERTY()
    TObjectPtr<ULevelSequencePlayer> SequencePlayer;

    UPROPERTY()
    TObjectPtr<ALevelSequenceActor> SequenceActor;

    UFUNCTION()
    void OnSequenceFinished();
	
};
