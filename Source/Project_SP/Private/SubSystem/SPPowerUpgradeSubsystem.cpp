// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPPowerUpgradeSubsystem.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Component/InventoryComponent.h"
#include "GameFramework/PlayerController.h"

void USPPowerUpgradeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FString AssetPath = TEXT("/Game/DataTable/DT_PowerUpgrade.DT_PowerUpgrade");

	UDataTable* LoadedTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *AssetPath));
	if (LoadedTable)
	{
		PowerUpgradeDataTable = LoadedTable;
		UE_LOG(LogTemp, Log, TEXT("✅ [PowerUpgrade] C++ 내부에서 데이터 테이블을 자동으로 찾아서 로드했습니다!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ [PowerUpgrade] 데이터 테이블 로드 실패! AssetPath 경로가 올바른지 확인하세요: %s"), *AssetPath);
	}
}

int32 USPPowerUpgradeSubsystem::GetPowerLevel(EPowerUpgradeType UpgradeType) const
{
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		const FPlayerMetaProgressionData& PermSaveData = SaveSys->GetPermData();

		//  권능 장부에서 현재 확인하려는 권능(UpgradeType)이 찍혀있는지 찾습니다.
		if (const int32* FoundLevel = PermSaveData.PowerUpgradeData.UpgradeLevels.Find(UpgradeType))
		{
			// 찾았다면 그 레벨을 반환
			return *FoundLevel;
		}
		
	}
	return 0; // 기록이 없으면 기본 0레벨
}

FPowerUpgradeData* USPPowerUpgradeSubsystem::GetUpgradeData(EPowerUpgradeType UpgradeType) const
{
	// 탐지 1: 게임 인스턴스에서 넘겨준 표가 아예 증발해버린 경우
	if (!PowerUpgradeDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ [PowerUpgrade] 테이블 에셋이 NULL입니다! (GameInstance의 Set 노드 값이 C++로 넘어오지 않고 휘발됨)"));
		return nullptr;
	}

	TArray<FPowerUpgradeData*> AllData;
	PowerUpgradeDataTable->GetAllRows<FPowerUpgradeData>(TEXT("PowerUpgradeContext"), AllData);

	// 탐지 2: 표는 잘 넘어왔는데, 안의 내용물(행)이 0개로 읽히는 경우
	if (AllData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ [PowerUpgrade] 테이블 연결은 성공했으나, 안의 데이터(행)가 0개로 인식됩니다!"));
		return nullptr;
	}

	// 일치하는 권능 찾기
	for (FPowerUpgradeData* Data : AllData)
	{
		if (Data && Data->UpgradeType == UpgradeType)
		{
			return Data; // 성공!
		}
	}

	// 탐지 3: 표도 있고 데이터도 있는데, 버튼에서 넘겨준 Enum과 표의 Enum이 일치하지 않는 경우
	UE_LOG(LogTemp, Error, TEXT("❌ [PowerUpgrade] 테이블 정상 (행 %d개). 하지만 표 안에서 [%d] 타입의 권능을 찾을 수 없습니다! Enum 엇갈림 발생."), AllData.Num(), (uint8)UpgradeType);
	return nullptr;
}

int32 USPPowerUpgradeSubsystem::GetNextLevelCost(EPowerUpgradeType UpgradeType) const
{
	FPowerUpgradeData* Data = GetUpgradeData(UpgradeType);
	if (!Data) return -1; // 구체적인 에러 로그는 위에서 이미 출력됨

	int32 CurrentLevel = GetPowerLevel(UpgradeType);

	// 탐지 4: 이미 최대 레벨인 경우
	if (CurrentLevel >= Data->MaxLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ [PowerUpgrade] 이미 설정된 최대 레벨(%d)에 도달했습니다!"), Data->MaxLevel);
		return -1;
	}

	if (!Data->CostPerLevel.IsValidIndex(CurrentLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ [PowerUpgrade] %d 레벨로 가기 위한 비용 데이터가 배열에 없습니다!"), CurrentLevel);
		return -1;
	}

	return Data->CostPerLevel[CurrentLevel];
}

bool USPPowerUpgradeSubsystem::TryUpgradePower(EPowerUpgradeType UpgradeType, APlayerController* PlayerController)
{
	if (!PlayerController) return false;

	int32 Cost = GetNextLevelCost(UpgradeType);
	if (Cost < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PowerUpgrade] 이미 최대 레벨이거나 비용 데이터가 없습니다."));
		return false;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn) return false;

	UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return false;

	// 핵심 기획: 오직 '파편'만 소모합니다! (이미 만들어두신 ConsumeFragment 함수 활용)
	if (!Inventory->ConsumeFragment(Cost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PowerUpgrade] 파편이 부족합니다! 필요 파편: %d"), Cost);
		return false;
	}

	// 파편 지불 성공 -> 레벨 증가 및 세이브 연동
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		int32 CurrentLevel = GetPowerLevel(UpgradeType);
		int32 NewLevel = CurrentLevel + 1;

		// 1. 권능 레벨 장부 업데이트 및 디스크 저장
		SaveSys->UpdatePowerUpgradeLevel(UpgradeType, NewLevel);

		// 🌟 2. 파편이 영구 재화라면, 캐릭터의 영구 데이터를 다시 캐싱하고 영구 디스크에 구워야 합니다!
		SaveSys->CachePermDataFromPlayer(PlayerPawn);
		SaveSys->SavePermToDisk();

		UE_LOG(LogTemp, Log, TEXT("[PowerUpgrade] 권능 수복 성공! [%d] 레벨 -> %d"), (int32)UpgradeType, NewLevel);
		return true;
	}

	return false;
}

float USPPowerUpgradeSubsystem::GetPowerEffectValue(EPowerUpgradeType UpgradeType) const
{
	int32 CurrentLevel = GetPowerLevel(UpgradeType);

	// 0레벨(안 찍음)이면 보너스 수치는 무조건 0입니다.
	if (CurrentLevel == 0) return 0.0f;

	FPowerUpgradeData* Data = GetUpgradeData(UpgradeType);
	if (!Data) return 0.0f;

	// 인덱스는 0부터 시작하므로, 1레벨 효과는 Index 0에 있습니다.
	int32 EffectIndex = CurrentLevel - 1;

	// 배열 범위 방어
	if (Data->EffectValuePerLevel.IsValidIndex(EffectIndex))
	{
		return Data->EffectValuePerLevel[EffectIndex];
	}

	return 0.0f;
}

void USPPowerUpgradeSubsystem::ApplyNewRunBonuses()
{
	USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>();
	if (!SaveSys) return;

	// 인벤토리가 주입해 둔 현재 지갑 데이터(500원)를 가져옵니다.
	FPlayerRunWallet DynamicWallet = SaveSys->GetRunData().RunWallet;

	// 영구 권능 레벨에 따른 보너스 골드 계산
	float BonusGold = GetPowerEffectValue(EPowerUpgradeType::StartGold);
	if (BonusGold > 0.0f)
	{
		// 🌟 덮어쓰기가 아니라 기존 데이터(500)에 보너스(100)를 더해줍니다 (500 + 100 = 600)
		DynamicWallet.Money += FMath::RoundToInt(BonusGold);
	}

	// 계산된 600원을 세이브 시스템에 다시 넣어줍니다.
	SaveSys->UpdateRunWalletData(DynamicWallet);
}
