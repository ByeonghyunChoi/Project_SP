// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPPowerUpgradeSubsystem.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Component/InventoryComponent.h"
#include "GameFramework/PlayerController.h"

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
	if (!PowerUpgradeDataTable) return nullptr;

	TArray<FPowerUpgradeData*> AllData;
	PowerUpgradeDataTable->GetAllRows<FPowerUpgradeData>(TEXT("PowerUpgradeContext"), AllData);

	for (FPowerUpgradeData* Data : AllData)
	{
		if (Data && Data->UpgradeType == UpgradeType) return Data;
	}
	return nullptr;
}

int32 USPPowerUpgradeSubsystem::GetNextLevelCost(EPowerUpgradeType UpgradeType) const
{
	FPowerUpgradeData* Data = GetUpgradeData(UpgradeType);
	if (!Data) return -1; // 데이터 없음 에러

	int32 CurrentLevel = GetPowerLevel(UpgradeType);

	// 최대 레벨 도달 검사
	if (CurrentLevel >= Data->MaxLevel) return -1;

	// 배열 인덱스 방어 (0레벨이면 0번 인덱스의 비용을 가져옴)
	if (Data->CostPerLevel.IsValidIndex(CurrentLevel))
	{
		return Data->CostPerLevel[CurrentLevel];
	}
	return -1;
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

		// 세이브 시스템의 전용 함수 호출 (데이터 무결성 유지!)
		SaveSys->UpdatePowerUpgradeLevel(UpgradeType, NewLevel);

		// 파편(런 재화)이 깎인 현재 상태를 즉시 런 세이브 파일에 저장 (어뷰징 원천 차단)
		SaveSys->CacheRunDataFromPlayer(PlayerPawn);
		SaveSys->SaveRunToDisk();

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

	// 1. 현재 세이브 시스템에 보관된 깨끗한(혹은 기존의) 런 지갑 데이터를 한 번 읽어옵니다.
	// (기존에 구현해 두신 const Getter 사용)
	FPlayerRunWallet DynamicWallet = SaveSys->GetRunData().RunWallet;

	// 2. [골드 보너스 계산] 시작 골드 권능 수치를 읽어와 지갑에 더해줍니다.
	float BonusGold = GetPowerEffectValue(EPowerUpgradeType::StartGold);
	if (BonusGold > 0.0f)
	{
		DynamicWallet.Money += FMath::RoundToInt(BonusGold);
	}

	// 3. [파편 보너스 계산] 나중에 "시작 파편 증가" 권능이 추가된다면? 세이브 시스템 수정 없이 여기에 그냥 한 줄만 추가하면 끝납니다!
	/*
	float BonusFragment = GetPowerEffectValue(EPowerUpgradeType::StartFragment);
	if (BonusFragment > 0.0f)
	{
		DynamicWallet.Fragment += FMath::RoundToInt(BonusFragment);
	}
	*/

	// 4. 모든 재화 계산이 끝난 최종 지갑 오브젝트를 세이브 시스템의 범용 창구로 슥 넘겨줍니다.
	SaveSys->UpdateRunWalletData(DynamicWallet);
}
