// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_Interact.h"
#include "Interface/InteractableInterface.h"
#include "Component/SPInteractionComponent.h"


USPGA_Interact::USPGA_Interact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USPGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!AvatarActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    USPInteractionComponent* InteractComp = AvatarActor->FindComponentByClass<USPInteractionComponent>();

    if (InteractComp)
    {
        AActor* TargetActor = InteractComp->GetCurrentInteractable();

        if (TargetActor && TargetActor->Implements<UInteractableInterface>())
        {
            IInteractableInterface* Interactable = Cast<IInteractableInterface>(TargetActor);

            if (Interactable && Interactable->CanInteract(Cast<APawn>(AvatarActor)))
            {
                Interactable->ExecuteInteraction(Cast<APawn>(AvatarActor));
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
