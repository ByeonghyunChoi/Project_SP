// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "EnterancePortal.generated.h"

UCLASS()
class PROJECT_SP_API AEnterancePortal : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	AEnterancePortal();

	virtual void ExecuteInteraction(AActor* Interactor) override;
	virtual FText GetInteractText() const override;
	virtual void PlayInteractSound_Implementation() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void ExecutePortalTransition();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

};
