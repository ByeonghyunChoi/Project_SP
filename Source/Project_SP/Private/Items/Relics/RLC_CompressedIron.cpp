// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Relics/RLC_CompressedIron.h"
#include "Component/AttributesComponent.h"
#include "Character/CombatPawn.h"

void URLC_CompressedIron::ApplyRelicEffect(AActor* Target)
{
    if (!Target) return;
    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
    {
        // 받는 피해량 7% 감소 (0.07f)
        Attr->ApplyRelicStats(ERelicStatType::DamageReduction, 0.07f);
        UE_LOG(LogTemp, Log, TEXT("[압축된 철괴] 받는 피해량이 감소했습니다."));
    }
}

void URLC_CompressedIron::RemoveRelicEffect(AActor* Target)
{
    if (!Target) return;
    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
    {
        Attr->ApplyRelicStats(ERelicStatType::DamageReduction, -0.07f);
    }
}

FStatModifiers URLC_CompressedIron::GetRelicModifiers() const
{
	FStatModifiers Mods;

	Mods.DamageReduction = 0.07f; // 받는 피해량 7% 감소

    return Mods;
}
