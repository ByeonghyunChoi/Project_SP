// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECT_SP_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//상호작용을 실행할 때 호출
	virtual void ExecuteInteraction(class ASPGASPlayerCharacter* Interactor) = 0;
	//UI에 표시할 상호작용 텍스트 반환
	virtual FText GetInteractText() = 0;
};
