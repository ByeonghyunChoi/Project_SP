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

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (USPSaveGameSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		// 🌟 [절대 방어벽]
		// 서브시스템(게임 인스턴스)은 맵이 넘어가도 죽지 않습니다.
		// "이번에 게임 켜고 초기화를 한 번이라도 했어?" 라고 물어봅니다.
		if (!SaveSys->bHasInitializedThisSession)
		{
			// 이 안으로 들어왔다는 것은 게임을 처음 켠 '첫 번째 맵'이라는 뜻입니다!
			// 여기서 치트키(bForceGiveTestCurrencies) 여부를 검사해서 재화를 줍니다.

			// 1. 영구 재화 주입
			if (!SaveSys->HasValidPermSave() || bForceGiveTestCurrencies)
			{
				PermanentWallet.Sand = 1000;
				PermanentWallet.IncompleteEnergy = 10;
				PermanentWallet.Fragment = 10; // 테스트용 파편 지급!

				if (OwnerPawn) SaveSys->CachePermDataFromPlayer(OwnerPawn);
				UE_LOG(LogTemp, Warning, TEXT("초기 영구 재화가 주입되었습니다."));
			}

			// 2. 런 재화 주입
			if (!SaveSys->HasValidRunSave() || bForceGiveTestCurrencies)
			{
				RunWallet.Money = 500;

				// 권능 보너스 골드 계산
				if (USPPowerUpgradeSubsystem* PowerSys = GetWorld()->GetGameInstance()->GetSubsystem<USPPowerUpgradeSubsystem>())
				{
					float BonusGold = PowerSys->GetPowerEffectValue(EPowerUpgradeType::StartGold);
					if (BonusGold > 0.0f)
					{
						RunWallet.Money += FMath::RoundToInt(BonusGold);
						UE_LOG(LogTemp, Log, TEXT("[Inventory] 권능 보너스 적용 완료! 최종 시작 골드: %d"), RunWallet.Money);
					}
				}

				if (OwnerPawn) SaveSys->CacheRunDataFromPlayer(OwnerPawn);
				UE_LOG(LogTemp, Warning, TEXT("초기 런 재화(골드)가 주입되었습니다."));
			}

			// 🌟 [가장 중요] 치트키로 재화를 줬든, 깡통으로 시작했든 세팅이 끝났습니다.
			// 셔터를 내려서 이번 세션(게임 끄기 전까지)에서는 다시는 이 코드가 실행되지 않게 막아버립니다!
			SaveSys->bHasInitializedThisSession = true;
		}
		else
		{
			// 맵을 이동해서 새로 태어난 경우, 이쪽으로 빠집니다.
			// 치트키가 true인 상태로 스폰되었더라도 절대 방어벽에 막혀버리므로, 
			// 우리가 의도한 대로 이전 맵에서 쓴 파편과 늘어난 600골드가 그대로 유지됩니다!
			UE_LOG(LogTemp, Log, TEXT("[Inventory] 이미 진행 중인 게임입니다. 치트키와 디버그 재화 초기화를 건너뜁니다."));
		}
	}

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
