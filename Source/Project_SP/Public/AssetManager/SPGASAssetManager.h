// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "SPGASAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_SP_API USPGASAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static USPGASAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
	
};
