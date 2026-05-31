// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Component/InventoryComponent.h"
#include "SubSystem/SPSaveGameSubsystem.h" //세이브 서브시스템 헤더
#include "SubSystem/SPPowerUpgradeSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (USPSaveGameSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		// 게임을 처음 켜서 시작한 첫 번째 맵(세션)일 때 딱 한 번만 권능 보너스를 정산합니다!
		if (!SaveSys->bHasInitializedThisSession)
		{
			// 🌟 권능 보너스 골드 계산 및 지급
			if (USPPowerUpgradeSubsystem* PowerSys = GetWorld()->GetGameInstance()->GetSubsystem<USPPowerUpgradeSubsystem>())
			{
				float BonusGold = PowerSys->GetPowerEffectValue(EPowerUpgradeType::StartGold);
				if (BonusGold > 0.0f)
				{
					RunWallet.Money += FMath::RoundToInt(BonusGold);
					UE_LOG(LogTemp, Log, TEXT("[Inventory] 권능 보너스 적용 완료! 보너스 골드: +%d"), FMath::RoundToInt(BonusGold));
				}
			}

			// 지급된 보너스 장부를 세이브 시스템 메모리에 안전하게 동기화!
			if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
			{
				SaveSys->CacheRunDataFromPlayer(OwnerPawn);
			}

			// 🌟 세팅이 끝났으니, 다음 맵부터는 이 코드가 다시 실행되지 않도록 스위치를 켭니다.
			SaveSys->ActivateNewRun();
			SaveSys->bHasInitializedThisSession = true;
		}
	}

	// 초기 UI 갱신 방송
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
	RunWallet.IncompleteEnergy += Amount;

	UE_LOG(LogTemp, Log, TEXT("기운 획득: +%d (현재: %d)"), Amount, RunWallet.IncompleteEnergy);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
}

bool UInventoryComponent::ConsumeIncompleteEnergy(int32 Amount)
{
	if (Amount <= 0) return false;
	if (RunWallet.IncompleteEnergy < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("기운 부족! 필요: %d, 보유: %d"), Amount, RunWallet.IncompleteEnergy);
		return false;
	}

	RunWallet.IncompleteEnergy -= Amount;
	UE_LOG(LogTemp, Log, TEXT("기운 소모: -%d (남은 양: %d)"), Amount, RunWallet.IncompleteEnergy);

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
	PermanentWallet.Fragment += Amount;

	UE_LOG(LogTemp, Log, TEXT("권능의 파편 획득: +%d (현재: %d)"), Amount, PermanentWallet.Fragment);
	if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(RunWallet, PermanentWallet);

	SyncWalletToSaveSystem();
}

bool UInventoryComponent::ConsumeFragment(int32 Amount)
{
	if (Amount <= 0) return false;
	if (PermanentWallet.Fragment < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("권능의 파편 부족! 필요: %d, 보유: %d"), Amount, PermanentWallet.Fragment);
		return false;
	}

	PermanentWallet.Fragment -= Amount;
	UE_LOG(LogTemp, Log, TEXT("권능의 파편 소모: -%d (남은 양: %d)"), Amount, PermanentWallet.Fragment);

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
