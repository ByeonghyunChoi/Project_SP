// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RelicBase.h"

void URelicBase::OnEquip(AActor* Instigator)
{
    if (Instigator)
    {
        ApplyRelicEffect(Instigator);
    }
}

void URelicBase::OnUnequip(AActor* Instigator)
{
    if (Instigator)
    {
        RemoveRelicEffect(Instigator);
    }
}

FStatModifiers URelicBase::GetRelicModifiers_Implementation() const
{
    // 기본형은 아무 스탯도 없는 빈 구조체 반환
    // (자식 BP나 C++ 클래스에서 이 함수를 오버라이드해서 실제 수치를 넣습니다)
    return FStatModifiers();
}

void URelicBase::ApplyRelicEffect(AActor* Target)
{

}

void URelicBase::RemoveRelicEffect(AActor* Target)
{

}
