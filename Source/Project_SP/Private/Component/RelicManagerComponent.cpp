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

    // 시작할 때 어트리뷰트 컴포넌트 찾아두기
    if (GetOwner())
    {
        AttributesComp = GetOwner()->FindComponentByClass<UAttributesComponent>();
    }
	
}

//void URelicManagerComponent::RecalculateRelicStats()
//{
//    if (!AttributesComp) return;
//
//    FStatModifiers TotalStats; // 합산용 임시 변수 (0으로 초기화됨)
//
//    // 1. 장착된 모든 유물 순회
//    for (const TObjectPtr<URelicBase>& Relic : EquippedRelics)
//    {
//        if (Relic)
//        {
//            // 각 유물에게 스탯 달라고 요청 (+) 연산자로 누적
//            TotalStats = TotalStats + Relic->GetRelicModifiers();
//        }
//    }
//
//    // 2. 합산된 최종 결과를 어트리뷰트 컴포넌트에 덮어쓰기
//    // (유물을 뺐을 때는 TotalStats가 줄어든 상태로 계산되므로 자동으로 감소 효과)
//    AttributesComp->UpdateRelicModifiers(TotalStats);
//
//    UE_LOG(LogTemp, Log, TEXT("유물 스탯 재계산 완료."));
//}

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
        NewRelic->OnEquip(GetOwner());

        // [추가] 유물이 늘어났으니 스탯 재계산!
        RecalculateRelicStats();
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
