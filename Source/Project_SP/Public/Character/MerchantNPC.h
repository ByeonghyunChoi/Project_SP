#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/InteractableInterface.h"
#include "MerchantNPC.generated.h"

class UShopComponent;

UCLASS()
class PROJECT_SP_API AMerchantNPC : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AMerchantNPC();

	//상호작용을 실행할 때 호출
	virtual void ExecuteInteraction(AActor* Interactor) override;
	//UI에 표시할 상호작용 텍스트 반환
	virtual FText GetInteractText() const override;

protected:
	// 1. 상점 기능을 담당할 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UShopComponent> ShopComp;

	// 2. 상인 이름 (예: "무기 상인", "잡화 상인") - 나중에 UI 제목으로 씁니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant Info")
	FText MerchantName;

protected:
	virtual void BeginPlay() override;

public:
	// 3. 외부(플레이어 컨트롤러)에서 호출할 함수
	// "상점 열어줘!"라고 명령하면 이 함수가 실행됩니다.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void OpenShop(APlayerController* PlayerController);
};