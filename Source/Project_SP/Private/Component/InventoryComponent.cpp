// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Component/InventoryComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h" //세이브 서브시스템 헤더
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (USPSaveGameSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		// 세이브 파일이 없거나, 디버그 플래그가 true일 때 작동!
		if (!SaveSys->HasValidPermSave() || bForceGiveTestCurrencies)
		{
			PermanentWallet.Sand = 1000;
			PermanentWallet.IncompleteEnergy = 10;

			if (OwnerPawn)
			{
				SaveSys->CachePermDataFromPlayer(OwnerPawn); // 영구 장부 즉시 동기화
			}
			UE_LOG(LogTemp, Warning, TEXT(" 초기 영구 재화가 주입되었습니다."));
		}

		//  세이브 파일이 없거나, 디버그 플래그가 true일 때 작동!
		if (!SaveSys->HasValidRunSave() || bForceGiveTestCurrencies)
		{
			RunWallet.Money = 500;
			RunWallet.Fragment = 10;

			if (OwnerPawn)
			{
				SaveSys->CacheRunDataFromPlayer(OwnerPawn); // 런 장부 즉시 동기화
			}
			UE_LOG(LogTemp, Warning, TEXT(" 초기 런 재화(골드)가 주입되었습니다."));
		}
		// 테스트 끝나면 헤더에 있는 bForceGiveTestCurrencies 변수도 같이 지워주기
	}

	// 초기 상태 UI 갱신
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);
}

void UInventoryComponent::AddSand(int32 Amount)
{
	if (Amount <= 0) return;
	PermanentWallet.Sand += Amount;

	UE_LOG(LogTemp, Log, TEXT("모래 획득: +%d (현재: %d)"), Amount, PermanentWallet.Sand);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
}

bool UInventoryComponent::ConsumeSand(int32 Amount)
{
	if (Amount <= 0) return false;
	if (PermanentWallet.Sand < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("모래 부족! 필요: %d, 보유: %d"), Amount, PermanentWallet.Sand);
		return false;
	}

	PermanentWallet.Sand -= Amount;
	UE_LOG(LogTemp, Log, TEXT("모래 소모: -%d (남은 양: %d)"), Amount, PermanentWallet.Sand);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
	return true;
}

void UInventoryComponent::AddIncompleteEnergy(int32 Amount)
{
	if (Amount <= 0) return;
	PermanentWallet.IncompleteEnergy += Amount;

	UE_LOG(LogTemp, Log, TEXT("기운 획득: +%d (현재: %d)"), Amount, PermanentWallet.IncompleteEnergy);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
}

bool UInventoryComponent::ConsumeIncompleteEnergy(int32 Amount)
{
	if (Amount <= 0) return false;
	if (PermanentWallet.IncompleteEnergy < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("기운 부족! 필요: %d, 보유: %d"), Amount, PermanentWallet.IncompleteEnergy);
		return false;
	}

	PermanentWallet.IncompleteEnergy -= Amount;
	UE_LOG(LogTemp, Log, TEXT("기운 소모: -%d (남은 양: %d)"), Amount, PermanentWallet.IncompleteEnergy);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
	return true;
}

void UInventoryComponent::AddMoney(int32 Amount)
{
	if (Amount <= 0) return;
	RunWallet.Money += Amount;

	UE_LOG(LogTemp, Log, TEXT("골드 획득: +%d (현재: %d)"), Amount, RunWallet.Money);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();

}

bool UInventoryComponent::ConsumeMoney(int32 Amount)
{
	if (Amount <= 0) return false;
	if (RunWallet.Money < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("골드 부족! 필요: %d, 보유: %d"), Amount, RunWallet.Money);
		return false;
	}

	RunWallet.Money -= Amount;
	UE_LOG(LogTemp, Log, TEXT("골드 소모: -%d (남은 양: %d)"), Amount, RunWallet.Money);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
	return true;
}

void UInventoryComponent::AddFragment(int32 Amount)
{
	if (Amount <= 0) return;
	RunWallet.Fragment += Amount;

	UE_LOG(LogTemp, Log, TEXT("권능의 파편 획득: +%d (현재: %d)"), Amount, RunWallet.Fragment);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
}

bool UInventoryComponent::ConsumeFragment(int32 Amount)
{
	if (Amount <= 0) return false;
	if (RunWallet.Fragment < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("권능의 파편 부족! 필요: %d, 보유: %d"), Amount, RunWallet.Fragment);
		return false;
	}

	RunWallet.Fragment -= Amount;
	UE_LOG(LogTemp, Log, TEXT("권능의 파편 소모: -%d (남은 양: %d)"), Amount, RunWallet.Fragment);

	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
	return true;
}

void UInventoryComponent::LoadWalletData(const FPlayerRunWallet& InRunWallet, const FPlayerPermanentWallet& InPermWallet)
{
	// 1. 세이브 파일에서 가져온 데이터로 내 지갑을 통째로 덮어씁니다.
	RunWallet = InRunWallet;
	PermanentWallet = InPermWallet;

	// 2. 돈이 바뀌었으니 UI(위젯) 숫자도 바뀌어야겠죠? 방송을 켭니다!
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);
	}

	UE_LOG(LogTemp, Log, TEXT("[Inventory] 지갑 복구 완료! 골드: %d / 모래: %d"), RunWallet.Money, PermanentWallet.Sand);
}

// 서브시스템 장부 갱신 함수 구현
void UInventoryComponent::SyncWalletToSaveSystem()
{
	// 인벤토리의 주인이 플레이어 폰인지 확인
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (USPSaveGameSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
		{
			// 디스크 저장이 아니라, 서브시스템의 메모리(Cache)만 즉시 갱신합니다! (렉 유발 X)
			SaveSys->CacheRunDataFromPlayer(OwnerPawn);
			SaveSys->CachePermDataFromPlayer(OwnerPawn);

			UE_LOG(LogTemp, Verbose, TEXT("[Inventory] 지갑 변동! 세이브 서브시스템 메모리에 즉시 동기화 완료."));
		}
	}
}
