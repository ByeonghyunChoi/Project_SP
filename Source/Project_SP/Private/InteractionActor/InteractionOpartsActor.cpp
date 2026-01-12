//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "InteractionActor/InteractionOpartsActor.h"
//#include "Character/SPGASPlayerCharacter.h"
//#include "Kismet/GameplayStatics.h"
//
//void AInteractionOpartsActor::BeginPlay()
//{
//	Super::BeginPlay();
//
//	if(OpartsWidgetClass)
//	{
//		UUserWidget* CreateOpartsWidget = CreateWidget<UUserWidget>(GetWorld(), OpartsWidgetClass);
//		OpartsWidgetInstance = CreateOpartsWidget;
//		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("OpartsWidgetInstance Created"));
//	}
//}
//
//void AInteractionOpartsActor::ExecuteInteraction(APlayerCharacter* Interactor)
//{
//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interaction"));
//	if (OpartsWidgetInstance)
//	{
//		OpartsWidgetInstance->AddToViewport();
//
//		FInputModeGameAndUI InputMode;
//		//Interactor->GetController<AMyPlayerController>()->SetInputMode(InputMode);
//	}
//}
//
//FText AInteractionOpartsActor::GetInteractText()
//{
//	return FText();
//}
