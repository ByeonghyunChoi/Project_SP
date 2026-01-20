// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "Items/Relics/RLC_StrengthAmulet.h"
//#include "Character/SPGASCharacterBase.h"
//
//void URLC_StrengthAmulet::ApplyRelicEffect(AActor* Target)
//{
//    if (!Target) return;
//    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
//    {
//        // 받는 피해량 7% 감소 (0.07f)
//        Attr->ApplyRelicStats(ERelicStatType::DamageIncrease, 0.07f);
//        UE_LOG(LogTemp, Log, TEXT("[힘의 부적] 가하는 피해량이 증가했습니다."));
//    }
//}
//
//void URLC_StrengthAmulet::RemoveRelicEffect(AActor* Target)
//{
//    if (!Target) return;
//    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
//    {
//        Attr->ApplyRelicStats(ERelicStatType::DamageIncrease, -0.07f);
//    }
//}
//
//FStatModifiers URLC_StrengthAmulet::GetRelicModifiers() const
//{
//    FStatModifiers Mods;
//
//	Mods.IncreaseDamage = 0.07f; // 가하는 피해량 7% 증가
//
//    return Mods;
//}
