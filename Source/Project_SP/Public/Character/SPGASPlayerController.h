// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "SPGASPlayerController.generated.h"

USTRUCT(BlueprintType)
struct FSPInputConfig
{
	GENERATED_BODY()

	//사용할 입력 액션
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UInputAction> InputAction;
	//해당 입력이 실행할 액션의 태그
	UPROPERTY(EditAnywhere)
	FGameplayTag InputTag; 
};

UCLASS()
class PROJECT_SP_API ASPGASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASPGASPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	void InitAbilitySystem(APawn* InPawn);

protected:
	//InputMappingContext 설정
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> FieldMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> BattleMappingContext;

	//필드용 InputAction 설정
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FSPInputConfig> FieldInputConfigs;

	//전투용 InputAction 설정
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FSPInputConfig> BattleInputConfigs;

	//이동은 GA를 사용하지 않기 때문에 따로 설정
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> MoveAction;

protected:
	void OnMove(const FInputActionValue& Value);
	void OnInputPressed(FGameplayTag InputTag);
	void OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount);

private:
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> CachedASC;

	
};
