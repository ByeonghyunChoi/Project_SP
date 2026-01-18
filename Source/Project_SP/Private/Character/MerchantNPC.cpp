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

// Called when the game starts or when spawned
void AMerchantNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMerchantNPC::OpenShop(APlayerController* PlayerController)
{
	if (!PlayerController) return;

	// [로그 확인] 
	// 아직 UI가 없으니, 로그로 기능이 작동하는지 먼저 확인
	UE_LOG(LogTemp, Warning, TEXT("상인 '%s'가 상점을 엽니다!"), *MerchantName.ToString());

	// 나중에 여기에 UI 생성 코드삽입.
	
}