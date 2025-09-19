// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemBase.h"
#include "Data/ArtifactData.h"
#include "ArtifactItem.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECT_SP_API UArtifactItem : public UItemBase
{
	GENERATED_BODY()

public:
	UArtifactItem();

	// 아티팩트의 고유 스탯 데이터를 담는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Artifact Data")
	FArtifactData ArtifactData;

	// 아티팩트 아이템을 초기화하고 데이터를 설정하는 함수
	UFUNCTION(BlueprintCallable, Category = "Artifact")
	void InitializeArtifact(FName InItemID, int32 InCount);

};
