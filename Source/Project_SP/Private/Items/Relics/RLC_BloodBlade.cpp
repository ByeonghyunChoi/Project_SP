// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "Items/Relics/RLC_BloodBlade.h"
//#include "Data/RelicData.h"
//
//void URLC_BloodBlade::ApplyRelicEffect(AActor* Target)
//{
//    if (!Target) return;
//
//    UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>();
//    if (Attr)
//    {
//        // [수정됨] ERelicStatType::AttackPower 사용
//        Attr->ApplyRelicStats(ERelicStatType::AttackPower, 0.1f);
//
//        UE_LOG(LogTemp, Log, TEXT("[핏빛 칼날] 공격력 10%% 증가"));
//    }
//}
//
//void URLC_BloodBlade::RemoveRelicEffect(AActor* Target)
//{
//    if (!Target) return;
//
//    UAttributesComponent* Attr = Target->FindComponentByClass<UAttributesComponent>();
//    if (Attr)
//    {
//        // 적용된 수치만큼 뺌
//        Attr->ApplyRelicStats(ERelicStatType::AttackPower, -0.1f);
//    }
//}
//
//FStatModifiers URLC_BloodBlade::GetRelicModifiers() const
//{
//	FStatModifiers Mods;
//
//	Mods.PercentAttack = 0.1f; // 공격력 10% 증가
//
//    return Mods;
//}
//
