#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h" 
#include "Map/MapInfo.h"
#include "PortalActor.generated.h"

UCLASS()
class PROJECT_SP_API APortalActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	APortalActor();

	// --- 인터페이스 구현 ---
	virtual void ExecuteInteraction(AActor* Interactor) override;
	virtual FText GetInteractText() const override;
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual void PlayInteractSound_Implementation() override;

	//매니저가 포탈을 생성한 후 목적지 타입을 설정해주는 함수
	void SetPortalTargetType(EMapType InType);

	//포탈을 활성화 하는 함수
	void ActivatePortal(bool bActive);

public:
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void ExecutePortalTransition();


protected:
	UPROPERTY(VisibleAnywhere, Category = "Portal | Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 이 포탈을 타면 이동하게 될 맵 유형
	UPROPERTY(VisibleAnywhere, Category = "Portal | Data")
	EMapType TargetMapType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bIsActive = false; // 기본은 비활성화

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal | Components")
	TObjectPtr<class UChildActorComponent> PortalVisualComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Portal | Visuals")
	TMap<EMapType, TSubclassOf<AActor>> PortalVisualClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Portal | UI")
	TMap<EMapType, FText> PortalInteractTextMap;

protected:
	// 활성화/비활성화 시 시각적 변화를 주기 위한 이벤트
	UFUNCTION(BlueprintImplementableEvent)
	void OnPortalStateChanged(bool bActive);

	//포탈의 색상을 바꾸는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Portal")
	void OnUpdatePortalColor(EMapType MapType);
};