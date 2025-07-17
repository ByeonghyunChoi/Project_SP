// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MonsterGroupObject.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class PROJECT_SP_API UMonsterGroupObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Group")
	TArray<TSubclassOf<class AMonsterCharacter>> MonsterClasses;
	
	UFUNCTION(BlueprintCallable, Category = "Monster Group")
	void CopyMonterGroup(UMonsterGroupObject* OtherGroup);
};
