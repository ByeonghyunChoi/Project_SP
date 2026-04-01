
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "InteractionActorBase.generated.h"

UCLASS()
class PROJECT_SP_API AInteractionActorBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	AInteractionActorBase();

protected:
	virtual void BeginPlay() override;

	virtual void ExecuteInteraction(class AActor* Interactor) override;

	virtual FText GetInteractText()const override;

};
