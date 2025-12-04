// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/RelicManagerComponent.h"
#include "Items/RelicBase.h"

URelicManagerComponent::URelicManagerComponent()
{

}

// Called when the game starts
void URelicManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void URelicManagerComponent::AddRelic(const FRelicData& NewRelicData)
{
    if (EquippedRelics.Num() >= MAX_RELIC_SLOTS)
    {
        UE_LOG(LogTemp, Warning, TEXT("유물 슬롯이 가득 찼습니다!"));
        return;
    }

    if (!NewRelicData.RelicClass) return;

    // 1. 유물 객체 생성 (Owner는 PlayerCharacter)
    URelicBase* NewRelic = NewObject<URelicBase>(GetOwner(), NewRelicData.RelicClass);

    if (NewRelic)
    {
        NewRelic->InitializeRelic(NewRelicData);

        EquippedRelics.Add(NewRelic);

        // 변경된 함수 호출 (GetOwner()를 넘겨줌)
        NewRelic->OnEquip(GetOwner());

        UE_LOG(LogTemp, Log, TEXT("유물 장착 완료: %s"), *NewRelic->GetName());
    }
}

// 모든 유물 초기화 함수
void URelicManagerComponent::ResetAllRelics()
{
    // 1. 장착된 모든 유물의 효과 제거 (스탯 원상복구)
    for (URelicBase* Relic : EquippedRelics)
    {
        if (Relic)
        {
            Relic->OnUnequip(GetOwner()); // 스탯 차감
        }
    }

    // 2. 배열 비우기
    EquippedRelics.Empty();

    UE_LOG(LogTemp, Log, TEXT("모든 유물이 초기화되었습니다."));
}

TArray<TSubclassOf<URelicBase>> URelicManagerComponent::GetEquippedRelicClasses() const
{
    TArray<TSubclassOf<URelicBase>> ResultClasses;

    for (const URelicBase* Relic : EquippedRelics)
    {
        if (Relic)
        {
            ResultClasses.Add(Relic->GetClass());
        }
    }
    return ResultClasses;
}

TArray<URelicBase*> URelicManagerComponent::GetEquippedRelics() const
{
    TArray<URelicBase*> Result;
    for (const TObjectPtr<URelicBase>& Relic : EquippedRelics)
    {
        if (Relic)
        {
            Result.Add(Relic.Get()); // 포인터 꺼내서 추가
        }
    }
    return Result;
}
