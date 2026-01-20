// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "Items/Relics/RLC_50YearTreeBranch.h"
//#include "Data/RelicData.h"
//
//void URLC_50YearTreeBranch::ApplyRelicEffect(AActor* Target)
//{
//    if (!Target) return;
//
//    UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>();
//    if (Attr)
//    {
//        // [수정됨] ERelicStatType::MaxHealth 사용
//        Attr->ApplyRelicStats(ERelicStatType::MaxHealth, 0.1f);
//
//        float HealthBonus = Attr->GetCurrentStats().fMaxHealth * 0.1f;
//        Attr->ApplyHealthChange(HealthBonus, Target);
//
//        UE_LOG(LogTemp, Log, TEXT("[고목나무 가지] 최대 체력이 증가했습니다 "));
//    }
//}
//
//void URLC_50YearTreeBranch::RemoveRelicEffect(AActor* Target)
//{
//    if (!Target) return;
//    if (UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>())
//    {
//        Attr->ApplyRelicStats(ERelicStatType::MaxHealth, -0.1f);
//    }
//}
//
//FStatModifiers URLC_50YearTreeBranch::GetRelicModifiers() const
//{ 
//	FStatModifiers Mods;
//
//	Mods.PercentHP = 0.1f; // 최대 체력 10% 증가
//
//    return Mods;
//}
//
