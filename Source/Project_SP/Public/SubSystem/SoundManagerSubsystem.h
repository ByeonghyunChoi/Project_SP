// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "SoundManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USoundManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 1. 초기화 및 정리
	virtual void Deinitialize() override;

	// 2. 외부에서 호출할 함수들
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayFieldBGM(USoundBase* NewBGM); // 필드 음악 재생

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SwitchToBattleBGM(USoundBase* BattleBGM); // 전투 음악으로 전환

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void ReturnToFieldBGM(); // 필드 음악으로 복귀

	//볼륨을 조절하는 함수
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetBGMVolume(float NewVolume);

	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetCurrentBGMVolume() const { return CurrentBGMVolume; }

protected:
	// 3. 실제 음악을 재생하는 스피커 2개
	UPROPERTY()
	TObjectPtr<UAudioComponent> FieldAudioComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BattleAudioComp;

	// 페이드 효과 시간 (초)
	const float FadeDuration = 1.0f;
	
	//현재 BGM 볼륨
	float CurrentBGMVolume = 1.0f;

	UPROPERTY()
	TObjectPtr<USoundBase> LastFieldBGMAsset;
};
