// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FPlayerMaterial
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Material")
	int32 Sand = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Material")
	int32 IncompleteEnergy = 0;
};

UCLASS()
class PROJECT_SP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// Sand (모래)
	UFUNCTION(BlueprintPure, Category = "Material")
	int32 GetCurrentSand() const;

	UFUNCTION(BlueprintCallable, Category = "Material")
	void SetCurrentSand(int32 NewAmount);

	// Incomplete Energy (불완전한 기운)
	UFUNCTION(BlueprintPure, Category = "Material")
	int32 GetCurrentIncompleteEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Material")
	void SetCurrentIncompleteEnergy(int32 NewAmount);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material")
	FPlayerMaterial PlayerMaterials;
};