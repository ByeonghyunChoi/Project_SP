// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPPowerUpgradeSubsystem.h"
#include "SubSystem/SPSaveGameSubsystem.h"
#include "Component/InventoryComponent.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AttributeSet/SPGASAttributeSet.h"

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

void USPPowerUpgradeSubsystem::GetStatUpgradeInfo(EPowerUpgradeType StatType, int32& OutCurrentLevel, int32& OutNextCost, float& OutCurrentEffectValue) const
{
	OutCurrentLevel = GetPowerLevel(StatType);

	// 1. 레벨별 요구 재화 (인덱스 0~4가 각각 1~5레벨로 가는 비용을 의미합니다)
	static const int32 UpgradeCosts[5] = { 10, 10, 15, 15, 20 };

	if (OutCurrentLevel < 5)
	{
		OutNextCost = UpgradeCosts[OutCurrentLevel];
	}
	else
	{
		OutNextCost = -1; // 만렙(5) 달성 시 -1을 반환하여 UI에서 MAX 처리 유도
	}

	// 2. 능력치별 1레벨당 증가량 세팅
	float IncreasePerLevel = 0.0f;
	switch (StatType)
	{
	case EPowerUpgradeType::Stat_HP:         IncreasePerLevel = 140.0f; break;
	case EPowerUpgradeType::Stat_ATK:        IncreasePerLevel = 70.0f;  break;
	case EPowerUpgradeType::Stat_DEF:        IncreasePerLevel = 40.0f;  break;
	case EPowerUpgradeType::Stat_Speed:      IncreasePerLevel = 15.0f;  break;
	case EPowerUpgradeType::Stat_DamageInc:  IncreasePerLevel = 10.0f;  break; // 10%
	case EPowerUpgradeType::Stat_CritChance: IncreasePerLevel = 5.0f;   break; // 5%
	case EPowerUpgradeType::Stat_CritDamage: IncreasePerLevel = 10.0f;  break; // 10%
	default: IncreasePerLevel = 0.0f; break;
	}

	// 3. 최종 효과 수치 = 현재 레벨 * 1레벨당 증가량
	OutCurrentEffectValue = OutCurrentLevel * IncreasePerLevel;
}

bool USPPowerUpgradeSubsystem::TryUpgradeStatInternal(EPowerUpgradeType StatType, APlayerController* PC)
{
	if (!PC) return false;

	int32 CurrentLevel, Cost;
	float CurrentEffect;
	GetStatUpgradeInfo(StatType, CurrentLevel, Cost, CurrentEffect);

	// 🌟 최대 레벨 3 -> 5로 방어 조건 수정
	if (CurrentLevel >= 5 || Cost < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatUpgrade] 이미 최대 레벨(5)입니다."));
		return false;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return false;

	UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return false;

	// 파편 차감
	if (!Inventory->ConsumeFragment(Cost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatUpgrade] 파편이 부족합니다! 필요: %d"), Cost);
		return false;
	}

	// 저장 및 동기화
	if (USPSaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USPSaveGameSubsystem>())
	{
		int32 NewLevel = CurrentLevel + 1;
		SaveSys->UpdatePowerUpgradeLevel(StatType, NewLevel);

		SaveSys->CachePermDataFromPlayer(PlayerPawn);
		SaveSys->SavePermToDisk();

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn);
		if (ASC)
		{
			FGameplayAttribute Attribute;
			float IncreaseValue = 0.0f;

			// ⚠️ 주의: GetHPAttribute() 등은 본인의 SPGASAttributeSet에 선언된 이름으로 꼭 맞춰주세요!
			// ⚠️ 퍼센트 수치의 경우, 기획 상 0.05 단위로 쓰신다면 5.0f 대신 0.05f 로 바꿔주셔야 합니다.
			switch (StatType)
			{
			case EPowerUpgradeType::Stat_HP:         Attribute = USPGASAttributeSet::GetMaxHealthAttribute(); IncreaseValue = 140.0f; break;
			case EPowerUpgradeType::Stat_ATK:        Attribute = USPGASAttributeSet::GetAttackAttribute(); IncreaseValue = 70.0f; break;
			case EPowerUpgradeType::Stat_DEF:        Attribute = USPGASAttributeSet::GetDefenseAttribute(); IncreaseValue = 40.0f; break;
			case EPowerUpgradeType::Stat_Speed:      Attribute = USPGASAttributeSet::GetSpeedAttribute(); IncreaseValue = 15.0f; break;
			case EPowerUpgradeType::Stat_DamageInc:  Attribute = USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute(); IncreaseValue = 0.10f; break;
			case EPowerUpgradeType::Stat_CritChance: Attribute = USPGASAttributeSet::GetCriticalRateAttribute(); IncreaseValue = 0.05f; break;
			case EPowerUpgradeType::Stat_CritDamage: Attribute = USPGASAttributeSet::GetCriticalDamageAttribute(); IncreaseValue = 0.10f; break;
			}

			if (Attribute.IsValid())
			{
				// 현재 베이스 수치를 가져와서 증가량을 더한 뒤 다시 덮어씌웁니다.
				float CurrentBase = ASC->GetNumericAttributeBase(Attribute);
				ASC->SetNumericAttributeBase(Attribute, CurrentBase + IncreaseValue);

				// 체력의 경우 최대 체력이 늘어났으니 현재 체력도 비율에 맞게(혹은 깡수치로) 채워주는 것이 자연스럽습니다.
				if (StatType == EPowerUpgradeType::Stat_HP)
				{
					float CurrentHP = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute()); // 현재 체력 변수명 확인 필요
					ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), CurrentHP + 140.0f);
				}

				UE_LOG(LogTemp, Warning, TEXT("[StatUpgrade] 실제 GAS 스탯 즉시 적용 완료! (+%f)"), IncreaseValue);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[StatUpgrade] 스탯 업그레이드 성공! [%d] %d -> %d 레벨"), (int32)StatType, CurrentLevel, NewLevel);
		return true;
	}

	return false;
}

// ---------------------------------------------------------
// 7개의 버튼 전용 함수들 (코드 중복을 막기 위해 Internal 함수로 전달)
// ---------------------------------------------------------

void USPPowerUpgradeSubsystem::UpgradeStat_ATK(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_ATK, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_ATK, OutLevel, OutNextCost, OutEffectValue);
}

void USPPowerUpgradeSubsystem::UpgradeStat_DEF(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_DEF, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_DEF, OutLevel, OutNextCost, OutEffectValue);
}

void USPPowerUpgradeSubsystem::UpgradeStat_Speed(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_Speed, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_Speed, OutLevel, OutNextCost, OutEffectValue);
}

void USPPowerUpgradeSubsystem::UpgradeStat_HP(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_HP, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_HP, OutLevel, OutNextCost, OutEffectValue);
}

void USPPowerUpgradeSubsystem::UpgradeStat_DamageInc(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_DamageInc, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_DamageInc, OutLevel, OutNextCost, OutEffectValue);
}

void USPPowerUpgradeSubsystem::UpgradeStat_CritChance(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_CritChance, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_CritChance, OutLevel, OutNextCost, OutEffectValue);
}

void USPPowerUpgradeSubsystem::UpgradeStat_CritDamage(APlayerController* PC, bool& bSuccess, int32& OutLevel, int32& OutNextCost, float& OutEffectValue)
{
	bSuccess = TryUpgradeStatInternal(EPowerUpgradeType::Stat_CritDamage, PC);
	GetStatUpgradeInfo(EPowerUpgradeType::Stat_CritDamage, OutLevel, OutNextCost, OutEffectValue);
}

// 새 런(게임) 시작 시 저장된 권능 보너스 일괄 적용
void USPPowerUpgradeSubsystem::ApplySavedStatUpgradesToPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn);
	if (!ASC) return;

	EPowerUpgradeType StatTypes[] = {
		EPowerUpgradeType::Stat_HP, EPowerUpgradeType::Stat_ATK, EPowerUpgradeType::Stat_DEF,
		EPowerUpgradeType::Stat_Speed, EPowerUpgradeType::Stat_DamageInc,
		EPowerUpgradeType::Stat_CritChance, EPowerUpgradeType::Stat_CritDamage
	};

	for (EPowerUpgradeType Type : StatTypes)
	{
		int32 Level = GetPowerLevel(Type);
		if (Level > 0)
		{
			float TotalBonus = 0.0f;
			FGameplayAttribute Attribute;

			switch (Type)
			{
			case EPowerUpgradeType::Stat_HP:         Attribute = USPGASAttributeSet::GetMaxHealthAttribute(); TotalBonus = Level * 140.0f; break;
			case EPowerUpgradeType::Stat_ATK:        Attribute = USPGASAttributeSet::GetAttackAttribute(); TotalBonus = Level * 70.0f; break;
			case EPowerUpgradeType::Stat_DEF:        Attribute = USPGASAttributeSet::GetDefenseAttribute(); TotalBonus = Level * 40.0f; break;
			case EPowerUpgradeType::Stat_Speed:      Attribute = USPGASAttributeSet::GetSpeedAttribute(); TotalBonus = Level * 15.0f; break;
			case EPowerUpgradeType::Stat_DamageInc:  Attribute = USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute(); TotalBonus = Level * 0.10f; break;
			case EPowerUpgradeType::Stat_CritChance: Attribute = USPGASAttributeSet::GetCriticalRateAttribute(); TotalBonus = Level * 0.05f; break;
			case EPowerUpgradeType::Stat_CritDamage: Attribute = USPGASAttributeSet::GetCriticalDamageAttribute(); TotalBonus = Level * 0.10f; break;
			}

			if (Attribute.IsValid())
			{
				float CurrentBase = ASC->GetNumericAttributeBase(Attribute);
				ASC->SetNumericAttributeBase(Attribute, CurrentBase + TotalBonus);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[StatUpgrade] 세이브 파일에서 읽어온 영구 스탯 보너스가 모두 적용되었습니다."));
}