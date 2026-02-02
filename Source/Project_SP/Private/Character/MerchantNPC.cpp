// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MerchantNPC.h"
#include "Component/ShopComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AMerchantNPC::AMerchantNPC()
{
	// 1. 상점 컴포넌트 생성 및 부착
	ShopComp = CreateDefaultSubobject<UShopComponent>(TEXT("ShopComponent"));

	// 2. 비주얼 설정 (일반적인 캐릭터 메시 방향 맞추기)
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

void AMerchantNPC::ExecuteInteraction(AActor* Interactor)
{
	// Interactor는 플레이어 캐릭터입니다.
	// 여기서 PlayerPawn->GetController()를 가져오는데, 이게 NULL일 수도 있습니다.
	if (APawn* PlayerPawn = Cast<APawn>(Interactor))
	{
		if (APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController()))
		{
			OpenShop(PC);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("상호작용 실패: 플레이어 컨트롤러를 찾을 수 없음!")); // [이게 뜨는지 확인]
		}
	}
}

FText AMerchantNPC::GetInteractText() const
{
	return FText::FromString(TEXT("상점 열기"));
}

// Called when the game starts or when spawned
void AMerchantNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMerchantNPC::OpenShop(APlayerController* PlayerController)
{
	if (!PlayerController) return;

	// 1. 상점 오픈 로그
	UE_LOG(LogTemp, Warning, TEXT("=== 상인 '%s'가 상점을 엽니다! ==="), *MerchantName.ToString());

	// 2. [검증] ShopComponent가 물건을 잘 가져오는지 테스트
	if (ShopComp)
	{
		TArray<FShopItemRow> Items = ShopComp->GetShopItems();

		if (Items.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT(">> 판매할 아이템이 없습니다! (데이터 테이블 연결 확인 필요)"));
		}
		else
		{
			for (const FShopItemRow& Item : Items)
			{
				UE_LOG(LogTemp, Log, TEXT(">> [판매 품목] ID: %s | 가격: %d | 재고: %d"), *Item.ItemID.ToString(), Item.Price, Item.Stock);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(">> ShopComponent가 없습니다!"));
	}

	// TODO: 나중에 여기에 UI 생성 코드(CreateWidget) 추가 예정
}