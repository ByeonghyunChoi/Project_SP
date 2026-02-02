// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SPInteractionComponent.h"
#include "Interface/InteractableInterface.h"
#include "UI/SPInteractionWidget.h"
#include "AbilitySystemBlueprintLibrary.h" 
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h" 
#include "DrawDebugHelpers.h"
#include "Project_SP/Project_SP.h"

USPInteractionComponent::USPInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void USPInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!InteractionWidgetInstance && InteractionWidgetClass)
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn && OwnerPawn->IsLocallyControlled())
		{
			APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
			if (PC)
			{
				InteractionWidgetInstance = CreateWidget<USPInteractionWidget>(PC, InteractionWidgetClass);
				if (InteractionWidgetInstance)
				{
					InteractionWidgetInstance->AddToViewport(); 
					InteractionWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}

	if (!InteractionWidgetInstance) return;

	//플레이어인 경우에만 감지 수행
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		PerformTrace();
	}
}

void USPInteractionComponent::PerformTrace()
{
	if (!CurrentInteractable.IsExplicitlyNull() && !CurrentInteractable.IsValid())
	{
		CurrentInteractable = nullptr; 
		if (InteractionWidgetInstance)
		{
			InteractionWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); // UI 즉시 끄기
		}
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;


	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector(); 
	FVector End = Start + (Forward * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner); 

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractionRadius);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, Start, End, FQuat::Identity,
		ECC_Interaction, SphereShape, Params
	);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (bShowDebug)
	{
		FColor DebugColor = bHit ? FColor::Green : FColor::Red;
		FVector Center = (Start + End) * 0.5f;
		float HalfHeight = (TraceDistance * 0.5f) + InteractionRadius;
		DrawDebugCapsule(GetWorld(), Center, HalfHeight, InteractionRadius,
			FRotationMatrix::MakeFromZ(Forward).ToQuat(), DebugColor, false, -1.0f);
	}
#endif

	AActor* NewTarget = nullptr;
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->Implements<UInteractableInterface>())
		{
			IInteractableInterface* Interface = Cast<IInteractableInterface>(HitActor);
			if (Interface)
			{
				if (Interface->CanInteract(Cast<APawn>(GetOwner())))
				{
					NewTarget = HitActor;
				}
				
			}
		}
	}

	// 상태 변경 시 처리
	if (CurrentInteractable.Get() != NewTarget)
	{
		CurrentInteractable = NewTarget;

		if (InteractionWidgetInstance)
		{
			if (NewTarget && NewTarget->Implements<UInteractableInterface>())
			{
				IInteractableInterface* Interface = Cast<IInteractableInterface>(NewTarget);
				if (Interface)
				{
					FText ActionText = Interface->GetInteractText();
					InteractionWidgetInstance->UpdateInteractionText(ActionText);
				}

				InteractionWidgetInstance->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				InteractionWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

