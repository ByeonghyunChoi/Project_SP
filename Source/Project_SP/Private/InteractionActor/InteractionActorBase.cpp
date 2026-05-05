

#include "InteractionActor/InteractionActorBase.h"

AInteractionActorBase::AInteractionActorBase()
{

}

void AInteractionActorBase::BeginPlay()
{
	Super::BeginPlay();

}

void AInteractionActorBase::ExecuteInteraction(AActor* Interactor)
{
	ReceiveInteraction(Interactor);
}

FText AInteractionActorBase::GetInteractText()const
{
	return InteractText;
}

