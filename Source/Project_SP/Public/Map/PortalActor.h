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

	/** @brief 매니저가 포탈을 생성한 후 목적지 타입을 설정해주는 함수 */
	void SetPortalTargetType(EMapType InType) { TargetMapType = InType; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Portal | Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 이 포탈을 타면 이동하게 될 맵 유형
	UPROPERTY(VisibleAnywhere, Category = "Portal | Data")
	EMapType TargetMapType;
};