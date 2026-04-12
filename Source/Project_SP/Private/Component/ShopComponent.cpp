// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ShopComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/DataTable.h"
#include "Data/ShopDataStructs.h"
#include "Data/Asset/RelicDefinition.h"
#include "Component/RelicComponent.h"
#include "Component/InventoryComponent.h"

UShopComponent::UShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShopComponent::BeginPlay()
{
	Super::BeginPlay();

	// 🌟 레벨이 시작될 때 딱 한 번! 데이터 테이블 원본을 내 주머니(RuntimeShopItems)로 복사해옵니다.
	if (ShopDataTable)
	{
		TArray<FShopItemRow*> AllRows;
		ShopDataTable->GetAllRows<FShopItemRow>(TEXT("ShopComponent Context"), AllRows);
		for (FShopItemRow* Row : AllRows)
		{
			if (Row)
			{
				RuntimeShopItems.Add(*Row);
			}
		}
	}
	GenerateRandomRelic();
	GenerateRandomCurrency();
}

FShopItemRow UShopComponent::GenerateRandomRelic()
{
	FShopItemRow NewRelicRow;

	if (RelicPoolDataTable && RuntimeShopItems.IsValidIndex(0))
	{
		// 1. 플레이어와 RelicComponent 가져오기
		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		URelicComponent* RelicComp = PlayerPawn ? PlayerPawn->FindComponentByClass<URelicComponent>() : nullptr;

		TArray<FRelicPoolRow*> AllRows;
		RelicPoolDataTable->GetAllRows<FRelicPoolRow>(TEXT(""), AllRows);

		// 2. 🌟 AcquiredHistory를 체크하여 중복 제거
		TArray<FRelicPoolRow*> AvailableRows;
		for (FRelicPoolRow* Row : AllRows)
		{
			if (Row && Row->RelicAsset)
			{
				// 성민 님의 헤더에 있는 AcquiredHistory 배열에 포함되어 있는지 확인
				if (RelicComp && !RelicComp->AcquiredHistory.Contains(Row->RelicAsset))
				{
					AvailableRows.Add(Row);
				}
			}
		}

		// 3. 필터링된 후보 중 랜덤 선택
		if (AvailableRows.Num() > 0)
		{
			int32 RandIdx = FMath::RandRange(0, AvailableRows.Num() - 1);
			FRelicPoolRow* Selected = AvailableRows[RandIdx];

			if (Selected && Selected->RelicAsset)
			{
				// 유물 에셋과 가격 정보를 ShopItemRow에 담아서 진열대(0번 슬롯)에 올려줍니다.
				NewRelicRow.RelicDataAsset = Selected->RelicAsset;

				NewRelicRow.DisplayName = Selected->RelicAsset->RelicName;
				NewRelicRow.Icon = Selected->RelicAsset->Icon;
				NewRelicRow.EffectClass = Selected->RelicAsset->RelicEffectClass;
				NewRelicRow.Price = Selected->Price;
				NewRelicRow.ItemID = Selected->RelicAsset->GetPrimaryAssetId().PrimaryAssetName;
				NewRelicRow.Stock = 1;

				RuntimeShopItems[0] = NewRelicRow;
			}
		}
		else
		{
			// 모든 유물을 획득해서 더 이상 팔 게 없을 때
			NewRelicRow.DisplayName = FText::FromString(TEXT("모든 유물 획득 완료"));
			NewRelicRow.Stock = 0;
			RuntimeShopItems[0] = NewRelicRow;
		}
	}
	return NewRelicRow;
}

void UShopComponent::ApplyRelicToPlayer(URelicDefinition* RelicAsset, AActor* TargetActor)
{
	if (!RelicAsset || !TargetActor) return;

	// 1. 플레이어에게서 RelicComponent를 찾습니다.
	URelicComponent* RelicComp = TargetActor->FindComponentByClass<URelicComponent>();

	if (RelicComp)
	{
		RelicComp->AddRelic(RelicAsset);

		UE_LOG(LogTemp, Warning, TEXT("Shop: [%s] 유물을 RelicComponent에 전달했습니다."), *RelicAsset->RelicName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Shop: 플레이어에게 RelicComponent가 없습니다!"));
	}
}

FShopItemRow UShopComponent::GenerateRandomCurrency()
{

	FShopItemRow NewRow;

	if (CurrencyDataTable && RuntimeShopItems.IsValidIndex(2))
	{
		TArray<FCurrencyShopRow*> AllCurrencies;
		CurrencyDataTable->GetAllRows<FCurrencyShopRow>(TEXT(""), AllCurrencies);

		if (AllCurrencies.Num() > 0)
		{
			int32 RandIdx = FMath::RandRange(0, AllCurrencies.Num() - 1);
			FCurrencyShopRow* Selected = AllCurrencies[RandIdx];

			if (Selected)
			{
				// UI 표시를 위해 "이름 x 수량" 형식으로 설정
				FString FullName = FString::Printf(TEXT("%s x%d"), *Selected->ItemName.ToString(), Selected->Amount);
				NewRow.DisplayName = FText::FromString(FullName);

				NewRow.Icon = Selected->Icon;
				NewRow.Price = Selected->Price;
				NewRow.CurrencyType = Selected->Type;
				NewRow.Amount = Selected->Amount;
				NewRow.Stock = 1;

				// ItemID에 타입을 저장해두면 나중에 지급할 때 편합니다.
				NewRow.ItemID = FName(*UEnum::GetValueAsString(Selected->Type));

				// 진짜 진열대(3번째 칸) 업데이트
				RuntimeShopItems[2] = NewRow;
			}
		}
	}
	return NewRow;
}


void UShopComponent::ConsumeStock(FName ItemID)
{
	// 🌟 주머니를 뒤져서 방금 산 아이템의 재고를 1 깎습니다.
	for (FShopItemRow& Item : RuntimeShopItems)
	{
		if (Item.ItemID == ItemID)
		{
			if (Item.Stock > 0)
			{
				Item.Stock--;
				UE_LOG(LogTemp, Warning, TEXT("[%s] 재고 감소! 남은 수량: %d"), *ItemID.ToString(), Item.Stock);
			}
			return; // 찾았으니 종료
		}
	}
}

void UShopComponent::GiveCurrencyToPlayer(ECurrencyType Type, int32 Amount, AActor* TargetActor)
{
	if (!TargetActor || Amount <= 0) return;

	UInventoryComponent* InventoryComp = TargetActor->FindComponentByClass<UInventoryComponent>();

	if (InventoryComp)
	{
		switch (Type)
		{
		case ECurrencyType::Energy:
			InventoryComp->AddIncompleteEnergy(Amount);
			break;
		case ECurrencyType::Sand:
			InventoryComp->AddSand(Amount);
			break;
		case ECurrencyType::Fragment:
			InventoryComp->AddFragment(Amount);
			break;
		}
		UE_LOG(LogTemp, Warning, TEXT("Shop: %d 개의 재화를 지급했습니다!"), Amount);
	}
}

void UShopComponent::ResetShopForNextLevel()
{
	// 1번 슬롯과 3번 슬롯의 ItemID를 초기화해서 다음 호출 때 새로 생성되게 만듭니다.
	if (RuntimeShopItems.IsValidIndex(0)) RuntimeShopItems[0].ItemID = NAME_None;
	if (RuntimeShopItems.IsValidIndex(2)) RuntimeShopItems[2].ItemID = NAME_None;

	// 🌟 만약 2번, 4번 고정 아이템들도 재고를 채우고 싶다면 여기서 처리합니다.
	for (FShopItemRow& Item : RuntimeShopItems)
	{
		Item.Stock = 1; // 또는 테이블 기본값으로 리셋
	}

	UE_LOG(LogTemp, Warning, TEXT("Shop: 다음 레벨을 위해 상점 재고가 초기화되었습니다."));
}