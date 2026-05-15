// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "SPHealingObject.generated.h"

UCLASS()
class PROJECT_SP_API ASPHealingObject : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:
	ASPHealingObject();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> RelicRewardWidgetClass;

public:
	virtual void ExecuteInteraction(AActor* Interactor) override;
	virtual FText GetInteractText() const override;
	virtual bool CanInteract(AActor* Interactor) const override { return true; }
	virtual void PlayInteractSound_Implementation() override;

};
