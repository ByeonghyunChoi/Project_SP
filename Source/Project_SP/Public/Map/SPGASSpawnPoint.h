// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "SPGASSpawnPoint.generated.h"

UCLASS()
class PROJECT_SP_API ASPGASSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
	ASPGASSpawnPoint();

public:
	// 스폰 위치 인덱스 (EncounterData의 SpawnPositionIndex와 매칭)
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn")
	int32 SpawnIndex = 0;

protected:
	// 에디터에서 화살표로 방향을 보기 위함
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif
};
