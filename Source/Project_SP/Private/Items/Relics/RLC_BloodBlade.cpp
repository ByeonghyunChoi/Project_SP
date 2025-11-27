// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Relics/RLC_BloodBlade.h"
#include "Component/AttributesComponent.h"
#include "Data/RelicData.h"

void URLC_BloodBlade::ApplyRelicEffect(AActor* Target)
{
    if (!Target) return;

    UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>();
    if (Attr)
    {
        // [¼öÁ¤µÊ] ERelicStatType::AttackPower »ç¿ë
        Attr->ApplyRelicStats(ERelicStatType::AttackPower, 0.1f);

        UE_LOG(LogTemp, Log, TEXT("[ÇÍºû Ä®³¯] °ø°Ý·Â 10%% Áõ°¡"));
    }
}

void URLC_BloodBlade::RemoveRelicEffect(AActor* Target)
{
    if (!Target) return;

    UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>();
    if (Attr)
    {
        // Àû¿ëµÈ ¼öÄ¡¸¸Å­ »­
        Attr->ApplyRelicStats(ERelicStatType::AttackPower, -0.1f);
    }
}