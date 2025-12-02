// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RelicManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API URelicManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URelicManagerComponent();

	// 유물 획득 함수 (UI에서 선택 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Relic")
	void AddRelic(TSubclassOf<URelicBase> RelicClass);

	// 모든 유물 초기화 함수 (새 게임 시작 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Relic")
	void ResetAllRelics();

	// 현재 장착된 유물 클래스 배열 반환
	UFUNCTION(BlueprintPure, Category = "Relic")
	TArray<TSubclassOf<URelicBase>> GetEquippedRelicClasses() const;

	// 현재 장착된 유물 개수 확인
	UFUNCTION(BlueprintPure)
	int32 GetRelicCount() const { return EquippedRelics.Num(); }

	const int32 MAX_RELIC_SLOTS = 3;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// 실제 생성된 유물 인스턴스 저장
	UPROPERTY(VisibleInstanceOnly, Category = "Relic")
	TArray<TObjectPtr<URelicBase>> EquippedRelics;
};
