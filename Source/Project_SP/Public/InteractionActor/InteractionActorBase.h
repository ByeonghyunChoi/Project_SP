
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oparts|Setup")
	FText InteractText = FText::FromString(TEXT("상호작용"));

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void ExecuteInteraction(class AActor* Interactor) override;

	virtual FText GetInteractText()const override;

	// 수신 받을때 실행될 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "On Interaction Received"))
	void ReceiveInteraction(class AActor* Interactor);

};
