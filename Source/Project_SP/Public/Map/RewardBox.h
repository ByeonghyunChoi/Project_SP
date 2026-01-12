//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//#include "Interface/InteractableInterface.h"
//#include "RewardBox.generated.h"
//
//class UDataTable;
//class inventoryComponent;
//class URelicSpawner;
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewardInteractedSignature);
//
//UCLASS()
//class PROJECT_SP_API ARewardBox : public AActor, public IInteractableInterface
//{
//	GENERATED_BODY()
//	
//public:	
//	// Sets default values for this actor's properties
//	ARewardBox();
//
//	//RewardBox Structure Section
//protected:
//	//박스 외형
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RewardBox")
//	TObjectPtr<UStaticMeshComponent> BoxMesh;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RewardBox")
//	TObjectPtr<UDataTable> RewardInfo;
//
//	// [추가] 유물 데이터 테이블 (InitializeReward에서 받아옴)
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RewardBox")
//	TObjectPtr<UDataTable> RelicDataTable;
//
//	//Interaction Logic Section
//public:
//	//상호작용 델리게이트
//	UPROPERTY(BlueprintAssignable, Category = "RewardBox")
//	FOnRewardInteractedSignature OnRewardInteracted;
//	//상호작용 로직
//	/*UFUNCTION(BlueprintCallable, Category = "RewardBox")
//	void PerformInteraction(ASPGASPlayerCharacter* Interactor);*/
//
//protected:
//	//상호작용 여부 판단(중복 방지)
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RewardBox")
//	bool bHasBeenInteracted = false;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RewardBox")
//	FName RewardRowName;
//
//	//Interface Logic Section
//public:
//	//플레이어가 상호작용 키를 눌러서 호출할 함수
//	virtual void ExecuteInteraction(APlayerCharacter* Interactor) override;
//
//	//보상 데이터 테이블과 로우 이름을 초기화하는 함수
//	void InitializeReward(UDataTable* InTable, FName InRowName, UDataTable* InRelicTable);
//
//	//플레이어 UI에 표시할 텍스트를 반환
//	UFUNCTION(BlueprintCallable)
//	virtual FText GetInteractText() override;
//
//protected:
//	void RewardToPlayer(APlayerCharacter* Interactor);
//
//	// [추가] 유물 보상 지급 로직
//	void TryGiveRelicReward(class APlayerCharacter* Interactor);
//};
