// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MerchantNPC.h"
#include "Component/ShopComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h" 
#include <Character/SPGASPlayerController.h>

// Sets default values
AMerchantNPC::AMerchantNPC()
{
	// 상점 컴포넌트 생성 및 부착
	ShopComp = CreateDefaultSubobject<UShopComponent>(TEXT("ShopComponent"));

	// 비주얼 설정
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

void AMerchantNPC::ExecuteInteraction(AActor* Interactor)
{
	if (APawn* PlayerPawn = Cast<APawn>(Interactor))
	{
		
		if (ASPGASPlayerController* PC = Cast<ASPGASPlayerController>(PlayerPawn->GetController()))
		{
			// "컨트롤러야, 너 지금 나(상인)랑 거래하는 거야" 라고 쥐여줍니다.
			PC->CurrentMerchant = this;
			OpenShop(PC);
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

	// 1. 기존의 상점 품목 로그 출력 (데이터 검증용으로 유지)
	UE_LOG(LogTemp, Warning, TEXT("=== 상인 '%s'가 상점을 엽니다! ==="), *MerchantName.ToString());

	if (ShopComp)
	{
		TArray<FShopItemRow> Items = ShopComp->RuntimeShopItems;
		for (const FShopItemRow& Item : Items)
		{
			UE_LOG(LogTemp, Log, TEXT(">> [판매 품목] ID: %s | 가격: %d"), *Item.ItemID.ToString(), Item.Price);
		}
	}

	// 2. UI 띄우기 로직
	if (ShopWidgetClass)
	{
		// 위젯이 아직 없다면 생성
		if (!ShopWidgetInstance)
		{
			ShopWidgetInstance = CreateWidget<UUserWidget>(PlayerController, ShopWidgetClass);
		}

		if (ShopWidgetInstance)
		{
			// 화면에 위젯 추가
			if (!ShopWidgetInstance->IsInViewport())
			{
				ShopWidgetInstance->AddToViewport();
			}

			// 입력 모드를 UI 전용으로 변경하고 마우스 커서를 보여줌
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ShopWidgetInstance->TakeWidget());

			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);

			UE_LOG(LogTemp, Warning, TEXT(">> 상점 UI가 성공적으로 열렸습니다."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(">> ShopWidgetClass가 설정되지 않았습니다!"));
	}
}