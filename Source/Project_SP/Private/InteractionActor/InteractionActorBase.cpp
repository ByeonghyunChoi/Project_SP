

#include "InteractionActor/InteractionActorBase.h"

AInteractionActorBase::AInteractionActorBase()
{

}

void AInteractionActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteractionActorBase::ExecuteInteraction(ASPGASPlayerCharacter* Interactor)
{

}

FText AInteractionActorBase::GetInteractText()
{
	return FText();
}

