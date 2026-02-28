// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "RelicDefinition.generated.h"

UENUM(BlueprintType)
enum class ERelicRarity : uint8
{
    Normal  UMETA(DisplayName = "일반"),
    Rare    UMETA(DisplayName = "희귀"),
    Unique  UMETA(DisplayName = "유니크")
};

UCLASS(BlueprintType, Const)
class PROJECT_SP_API URelicDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display")
	FText RelicName; // 유물 이름

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display")
	UTexture2D* Icon; // 유물 아이콘

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "1. Display", meta = (MultiLine = true))
	FText Description; // 유물 설명

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "2. Info")
	ERelicRarity Rarity; // 유물 희귀도

    //장착 시 플레이어에게 부여할 패시브 스탯/능력 이펙트
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "3. Ability")
    TSubclassOf<UGameplayEffect> RelicEffectClass;
};
