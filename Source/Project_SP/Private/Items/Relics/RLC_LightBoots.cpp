// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Relics/RLC_LightBoots.h"
#include "Component/AttributesComponent.h"
#include "Character/CombatPawn.h"

void URLC_LightBoots::ApplyRelicEffect(AActor* Target)
{
    if (!Target) return;
    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
    {
        // 속도 10% 증가 (0.1f)
        Attr->ApplyRelicStats(ERelicStatType::MovementSpeed, 0.1f);
        UE_LOG(LogTemp, Log, TEXT("[빛의 장화] 이동 속도가 증가했습니다."));
    }
}

void URLC_LightBoots::RemoveRelicEffect(AActor* Target)
{
    if (!Target) return;
    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
    {
        // 증가분 제거 (-0.1f)
        Attr->ApplyRelicStats(ERelicStatType::MovementSpeed, -0.1f);
    }
}

FStatModifiers URLC_LightBoots::GetRelicModifiers() const
{
	FStatModifiers Mods;

	Mods.PercentSpeed = 0.1f; // 이동 속도 10% 증가

    return Mods;
}
