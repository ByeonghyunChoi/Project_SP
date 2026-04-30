// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tag/SPGameplayTags.h"
#include "SPBattleDirector.generated.h"

class ASPBattleCameraActor;
class USPStatusEffectComponent;

USTRUCT(BlueprintType)
struct FStatusVisualRequest
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* TargetActor = nullptr;

	UPROPERTY()
	FGameplayTag StatusTag;

	UPROPERTY()
	bool bIsInstant = false;
};

UCLASS()
class PROJECT_SP_API ASPBattleDirector : public AActor
{
	GENERATED_BODY()
	
public:
	ASPBattleDirector();

	// 카메라 매니저 참조 받기
	UFUNCTION(BlueprintCallable, Category = "Battle | Visual")
	void InitDirector(ASPBattleCameraActor* InCameraManager);

	// 몬스터가 스폰될 때마다 호출하여 방송(Delegate)을 수신하도록 연결
	UFUNCTION(BlueprintCallable, Category = "Battle | Visual")
	void RegisterMonster(AActor* MonsterActor);

	UFUNCTION(BlueprintCallable, Category = "Battle | Visual")
	void FinishStatusSequence();

protected:
	// 상태이상 컴포넌트의 방송을 수신할 C++ 함수
	UFUNCTION()
	void HandleStatusVisual(AActor* TargetActor, FGameplayTag StatusTag, bool bIsInstant);

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle | Visual")
	void OnTargetFocusBegin(AActor* TargetActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle | Visual")
	void PlayStatusSequence(AActor* TargetActor, FGameplayTag StatusTag, bool bIsInstant);

	UFUNCTION(BlueprintImplementableEvent, Category = "BattleDirector|Visual")
	void PlayInstantStatusSequence(AActor* TargetActor, FGameplayTag StatusTag);

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle | Visual")
	void OnTargetFocusEnd(AActor* TargetActor);

	void TryPlayNextVisual();

protected:
	// 카메라 매니저 조종 리모컨 (블루프린트에서 꺼내 쓸 수 있게 만듦)
	UPROPERTY(BlueprintReadOnly, Category = "Battle | Camera")
	TObjectPtr<ASPBattleCameraActor> CameraManager;

	UPROPERTY()
	TArray<FStatusVisualRequest> VisualQueue;

	UPROPERTY()
	bool bIsPlayingVisual = false;

	// 현재 카메라가 비추고 있는 타겟 기억하기
	UPROPERTY()
	TObjectPtr<AActor> CurrentFocusedActor = nullptr;

	// 현재 연출이 진행되고 있는 타겟
	UPROPERTY()
	AActor* CurrentPlayingActor = nullptr;

};
