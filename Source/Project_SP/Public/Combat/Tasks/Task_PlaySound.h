// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatTask.h"
#include "Task_PlaySound.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS()
class PROJECT_SP_API UTask_PlaySound : public UCombatTask
{
	GENERATED_BODY()

protected:
    /** 이 태스크에서 재생할 사운드 애셋입니다. */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    TObjectPtr<USoundBase> SoundToPlay;

    /** true이면 2D (UI처럼)로 재생하고, false이면 Instigator의 위치에서 3D로 재생합니다. */
    UPROPERTY(EditAnywhere, Category = "Task Properties")
    bool bPlayAs2D = false;

public:
    virtual void ExecuteTask_Implementation() override;
	
};
