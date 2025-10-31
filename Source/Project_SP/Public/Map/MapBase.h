// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/CombatTypes.h"
#include "MapBase.generated.h"

UCLASS()
class PROJECT_SP_API AMapBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Map Structure Section
protected:
	//¸Ê Á¾·ù
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Type")
	EMapType CurrentMapType;
	//¸Ê »óÅÂ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMapState CurrentMapState;
	//½ºÆùÇÑ ¸Ê ¾×ÅÍ
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Map")
	TSubclassOf<AActor> MapActor;

public:
	void SetMapType(const EMapType& NewMapType);
	EMapType GetMapType() const;

	void SetMapState(const EMapState& NewMapState);
	EMapState GetMapState() const;

};
