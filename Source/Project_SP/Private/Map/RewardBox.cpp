// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/RewardBox.h"
#include "Data/RewardData.h"
#include "Data/RelicData.h" 
#include "Items/RelicSpawner.h"   // 스포너
#include "Component/RelicManagerComponent.h"
#include "Character/MyPlayerController.h"
#include "Character/PlayerCharacter.h"
#include "Component/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ARewardBox::ARewardBox()
{
    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    SetRootComponent(BoxMesh);

    bHasBeenInteracted = false;
}

void ARewardBox::PerformInteraction(APlayerCharacter* Interactor)
{
    if (bHasBeenInteracted)
    {
        return;
    }

    bHasBeenInteracted = true;

    OnRewardInteracted.Broadcast();
    SetActorEnableCollision(false);

    //보상을 주는 로직을 여기에 구현
    RewardToPlayer(Interactor);

    // [2] 유물 지급 시도 (신규 로직)
    TryGiveRelicReward(Interactor);

    this->Destroy();
}

void ARewardBox::ExecuteInteraction(APlayerCharacter* Interactor)
{
    PerformInteraction(Interactor);
}

void ARewardBox::InitializeReward(UDataTable* InTable, FName InRowName, UDataTable* InRelicTable)
{
    RewardInfo = InTable;
	RewardRowName = InRowName;
    RelicDataTable = InRelicTable;
}

FText ARewardBox::GetInteractText()
{
    return FText::FromString(TEXT("보상 열기[F]"));
}

void ARewardBox::RewardToPlayer(APlayerCharacter* Interactor)
{
    if (!RewardInfo || RewardRowName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("보상 데이터가 설정되지 않았습니다!"));
        return;
    }
    static const FString ContextString(TEXT("Reward Box Context"));
    FRewardData* RewardRow = RewardInfo->FindRow<FRewardData>(RewardRowName, ContextString);

    if (!RewardRow)
    {
        UE_LOG(LogTemp, Error, TEXT("RowName [%s]을 찾을 수 없습니다!"), *RewardRowName.ToString());
        return;
    }

	UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();
	
    if (InventoryComp)
    {
        InventoryComp->GainSand(RewardRow->SandAmount);
        InventoryComp->GainIncompleteEnergy(RewardRow->IncompleteEnergyAmount);
        InventoryComp->GainMoney(RewardRow->MoneyAmount);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("플레이어의 인벤토리 컴포넌트를 찾을 수 없습니다!"));
    }
}

void ARewardBox::TryGiveRelicReward(APlayerCharacter* Interactor)
{

    UE_LOG(LogTemp, Log, TEXT("유물 얻기를 시도 했습니다."));
    if (!RewardInfo || !RelicDataTable || RewardRowName.IsNone())
    {
        UE_LOG(LogTemp, Log, TEXT("보상정보 혹은 유물정보 혹은 행이름을 가져오지 못했습니다."));
        return;
    }

    // 보상 데이터 확인 (이 상자가 유물을 주는지?)
    static const FString ContextString(TEXT("RewardBoxContext"));
    FRewardData* RewardRow = RewardInfo->FindRow<FRewardData>(RewardRowName, ContextString);

    // 유물 보상이 포함된 경우에만 실행
    if (RewardRow && RewardRow->bHasRelicReward)
    {
        UE_LOG(LogTemp, Log, TEXT("유물을 주는 상자입니다."));
        // 필요한 컴포넌트 가져오기
        URelicManagerComponent* RelicMgr = Interactor->FindComponentByClass<URelicManagerComponent>();
        AMyPlayerController* PC = Cast<AMyPlayerController>(Interactor->GetController());

        if (RelicMgr && PC)
        {
            UE_LOG(LogTemp, Log, TEXT("컴포넌트들을 무사히 가져옴"));
            // 장착 중인 유물 클래스 가져오기 (중복 방지)
            TArray<TSubclassOf<URelicBase>> EquippedClasses = RelicMgr->GetEquippedRelicClasses();

            // 스포너 생성
            URelicSpawner* Spawner = NewObject<URelicSpawner>(this);

            // 랜덤 선택지 3개 뽑기
            TArray<FRelicData> Choices = Spawner->GenerateRandomRelicChoices(RelicDataTable, EquippedClasses);

            // 컨트롤러에게 UI 표시 요청
            if (Choices.Num() > 0)
            {
                PC->ShowRelicSelectionUI(Choices);
                UE_LOG(LogTemp, Log, TEXT("보상 상자: 유물 선택 UI를 호출했습니다."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("유물을 주지 않는 상자입니다."));
    }

}



