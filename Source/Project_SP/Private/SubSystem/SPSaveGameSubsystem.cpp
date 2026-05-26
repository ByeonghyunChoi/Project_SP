// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPSaveGameSubsystem.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/RelicComponent.h"
#include "Component/OpartsComponent.h"
#include "Map/MapManagerSubSystem.h"
#include "Save/SPRunSaveGame.h"
#include "Save/SPPermSaveGame.h"
#include "Kismet/GameplayStatics.h"

void USPSaveGameSubsystem::CacheRunDataFromPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	// 1. 스탯 저장
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			bool bFound = false;
			RunData.Stats.CurrentHealth = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetHealthAttribute(), bFound);
			RunData.Stats.CurrentBattlePoint = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute());
			RunData.Stats.CurrentTimePower = ASC->GetGameplayAttributeValue(USPGASAttributeSet::GetTimePowerAttribute(), bFound);
			RunData.Stats.CurrentActionGauge = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute());

			RunData.Stats.MaxHealth = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxHealthAttribute());
			RunData.Stats.MaxBattlePoint = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxBattlePointAttribute());
			RunData.Stats.MaxTimePower = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxTimePowerAttribute());

			RunData.Stats.Attack = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetAttackAttribute());
			RunData.Stats.Defense = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDefenseAttribute());
			RunData.Stats.Speed = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetSpeedAttribute());

			RunData.Stats.DefenseIgnore = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetDefenseIgnoreAttribute());
			RunData.Stats.CriticalRate = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCriticalRateAttribute());
			RunData.Stats.CriticalDamage = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetCriticalDamageAttribute());
			RunData.Stats.EffectHitRate = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectHitRateAttribute());
			RunData.Stats.EffectAmplify = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetEffectAmplifyAttribute());

			RunData.Stats.OutgoingDamageMultiplier = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetOutgoingDamageMultiplierAttribute());
			RunData.Stats.IncomingDamageMultiplier = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetIncomingDamageMultiplierAttribute());
			RunData.Stats.Level = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetLevelAttribute());
			RunData.Stats.Experience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute());
			RunData.Stats.MaxExperience = ASC->GetNumericAttributeBase(USPGASAttributeSet::GetMaxExperienceAttribute());
		}
	}

	// 2. 런 지갑 (골드, 파편) 저장
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		RunData.RunWallet.Money = InventoryComp->GetMoney();
	}

	// 3. 유물 저장
	if (URelicComponent* RelicComp = PlayerPawn->FindComponentByClass<URelicComponent>())
	{
		RunData.RelicData.EquippedRelics = RelicComp->EquippedRelics;
		RunData.RelicData.AcquiredHistory = RelicComp->AcquiredHistory;
	}

	// 4. 맵 진행도 저장
	if (UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>())
	{
		// 맵 매니저가 들고 있는 진행 정보 캐싱
		RunData.MapProgress.CurrentStage = MapManager->GetCurrentStage();
		RunData.MapProgress.CurrentFloor = MapManager->GetCurrentFloor();
		RunData.MapProgress.CurrentMapType = MapManager->GetCurrentMapType();
		RunData.MapProgress.CurrentRoomState = MapManager->GetCurrentRoomState();
		RunData.MapProgress.SavedPortalOptions = MapManager->GetCurrentPortalOptions();

		// 로비 맵인지 확인 정보 캐싱
		RunData.MapProgress.bIsSavedInLobby = MapManager->GetIsInLobby();
		// 플레이어의 현재 실제 위치 캐싱
		RunData.MapProgress.SavedPlayerTransform = PlayerPawn->GetActorTransform();

		RunData.MapProgress.PreGeneratedEncounters = MapManager->GetPreGeneratedEncounters();
	}
}

void USPSaveGameSubsystem::RestoreRunDataToPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || !RunData.IsValid()) return;

	// 스탯 복구
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			// 경험치 복구
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), RunData.Stats.Experience);

			// 맵 매니저를 통해 로비인지 확인
			UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();

			if (MapManager && MapManager->GetIsInLobby())
			{
				float FinalMaxHP = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
				float FinalMaxTP = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxTimePowerAttribute());

				// [로비일 때] 데이터 초기화
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), FinalMaxHP);
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), FinalMaxTP);
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), 0.0f);

				// 런타임 데이터에 덮어쓰기
				RunData.Stats.CurrentHealth = FinalMaxHP;
				RunData.Stats.CurrentTimePower = FinalMaxTP;
				RunData.Stats.CurrentActionGauge = 0.0f;
			}
			else
			{
				// [던전/전투일 때] 세이브 파일에 기록된 현재 상태를 그대로 불러옵니다.
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), RunData.Stats.CurrentHealth);
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetBattlePointAttribute(), RunData.Stats.CurrentBattlePoint);
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), RunData.Stats.CurrentTimePower);
				ASC->SetNumericAttributeBase(USPGASAttributeSet::GetActionGaugeAttribute(), RunData.Stats.CurrentActionGauge);
			}
		}
	}

	// 유물 복구 (스탯 기반으로 적용되므로 스탯 이후에 호출)
	if (URelicComponent* RelicComp = PlayerPawn->FindComponentByClass<URelicComponent>())
	{
		RelicComp->LoadRelicData(RunData.RelicData);
	}

	// 런/영구 지갑 일괄 세팅 (UI 갱신을 위해 한 번에 넘겨줌)
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		InventoryComp->LoadWalletData(RunData.RunWallet, PermData.PermanentWallet);
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 런 데이터 복구 완료!"));
}

void USPSaveGameSubsystem::ResetRunData()
{
	RunData.Reset();
	if (UGameplayStatics::DoesSaveGameExist(RunSlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(RunSlotName, 0);
	}
	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 사망/클리어: 런(Run) 데이터만 초기화되었습니다."));
}

void USPSaveGameSubsystem::CachePermDataFromPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	// 1. 영구 지갑 저장
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		PermData.PermanentWallet.Fragment = InventoryComp->GetFragment();
		PermData.PermanentWallet.Sand = InventoryComp->GetSand();
		PermData.PermanentWallet.IncompleteEnergy = InventoryComp->GetIncompleteEnergy();
	}

	// 2. 오파츠 저장
	if (UOpartsComponent* OpartsComp = PlayerPawn->FindComponentByClass<UOpartsComponent>())
	{
		PermData.OpartsData.EquippedOparts = OpartsComp->GetCurrentOpartsData().Definition;
		PermData.OpartsData.ProgressMap = OpartsComp->OpartsProgressMap;
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 영구 데이터 캐싱 완료!"));
}

void USPSaveGameSubsystem::RestorePermDataToPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;

	// 1. 오파츠 복구
	if (UOpartsComponent* OpartsComp = PlayerPawn->FindComponentByClass<UOpartsComponent>())
	{
		OpartsComp->LoadOpartsData(PermData.OpartsData);
	}

	// 2. 지갑 복구 (로비 진입 시 영구 데이터만 먼저 불릴 수 있으므로 여기서도 세팅)
	if (UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		InventoryComp->LoadWalletData(RunData.RunWallet, PermData.PermanentWallet);
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 영구 데이터 복구 완료!"));
}

void USPSaveGameSubsystem::ResetAllData()
{
	RunData.Reset();
	PermData.Reset();

	if (UGameplayStatics::DoesSaveGameExist(RunSlotName, 0)) UGameplayStatics::DeleteGameInSlot(RunSlotName, 0);
	if (UGameplayStatics::DoesSaveGameExist(PermSlotName, 0)) UGameplayStatics::DeleteGameInSlot(PermSlotName, 0);

	UE_LOG(LogTemp, Warning, TEXT("[SaveSystem] 새로하기: 영구 데이터와 런 데이터가 모두 공장 초기화되었습니다! (Hard Reset)"));
}


void USPSaveGameSubsystem::UpdatePowerUpgradeLevel(EPowerUpgradeType UpgradeType, int32 NewLevel)
{
	// 1. 자신의 영구 데이터 장부에 새로운 레벨을 기록합니다.
	PermData.PowerUpgradeData.UpgradeLevels.Add(UpgradeType, NewLevel);

	// 2. 변경된 내용이 유실되지 않게 즉시 디스크(Slot_Perm)에 덮어씁니다.
	SavePermToDisk();

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 권능 데이터 업데이트 및 영구 저장 완료! (%d -> 레벨 %d)"), (uint8)UpgradeType, NewLevel);
}

void USPSaveGameSubsystem::UpdateRunWalletData(const FPlayerRunWallet& NewWallet)
{
	// 외부(권능 매니저 등)에서 완전히 계산되어 넘어온 지갑 데이터를 그대로 덮어씁니다.
	RunData.RunWallet = NewWallet;

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 런 지갑 데이터가 범용 창구를 통해 성공적으로 업데이트되었습니다. (Money: %d)"),RunData.RunWallet.Money);
}

void USPSaveGameSubsystem::ActivateNewRun()
{
	RunData.bIsRunStarted = true; // 스위치 ON!
}

void USPSaveGameSubsystem::SaveRunToDisk()
{
	USPRunSaveGame* SaveInst = Cast<USPRunSaveGame>(UGameplayStatics::CreateSaveGameObject(USPRunSaveGame::StaticClass()));
	if (SaveInst)
	{
		SaveInst->RunData = RunData; // 서브시스템 메모리 데이터를 껍데기에 복사
		UGameplayStatics::SaveGameToSlot(SaveInst, RunSlotName, 0); // 디스크에 굽기!
		UE_LOG(LogTemp, Warning, TEXT("런 데이터 디스크 저장 완료! [%s]"), *RunSlotName);
	}
}

bool USPSaveGameSubsystem::LoadRunFromDisk()
{
	if (UGameplayStatics::DoesSaveGameExist(RunSlotName, 0))
	{
		USPRunSaveGame* LoadInst = Cast<USPRunSaveGame>(UGameplayStatics::LoadGameFromSlot(RunSlotName, 0));
		if (LoadInst)
		{
			RunData = LoadInst->RunData; // 디스크 데이터를 서브시스템 메모리로 가져오기
			UE_LOG(LogTemp, Warning, TEXT("런 데이터 디스크 로드 성공!"));
			return true;
		}
	}
	return false;
}

bool USPSaveGameSubsystem::HasValidPermSave() const
{
	return UGameplayStatics::DoesSaveGameExist(PermSlotName, 0);
}

bool USPSaveGameSubsystem::HasValidRunSave() const
{
	return UGameplayStatics::DoesSaveGameExist(RunSlotName, 0);
}

void USPSaveGameSubsystem::SaveSoundSettings(float Master, float BGM, float SFX, float UI)
{
	// 1. 메모리의 영구 데이터에 덮어쓰기
	PermData.SoundSettings.MasterVolume = Master;
	PermData.SoundSettings.BGMVolume = BGM;
	PermData.SoundSettings.SFXVolume = SFX;
	PermData.SoundSettings.UIVolume = UI;

	// 2. 변경된 영구 데이터를 즉시 디스크에 저장!
	SavePermToDisk();

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 사운드 설정 영구 데이터에 저장 완료! (Master: %f)"), Master);
}

void USPSaveGameSubsystem::MarkIntroAsSeen()
{
	// 오프닝을 봤다고 체크!
	PermData.bHasSeenIntro = true;

	// 이 중요한 사실을 잊어버리지 않게 즉시 디스크(영구 세이브)에 덮어씁니다.
	SavePermToDisk();

	UE_LOG(LogTemp, Warning, TEXT("[SaveSystem] 플레이어가 최초 오프닝을 시청했습니다. 영구 저장 완료!"));
}

void USPSaveGameSubsystem::MarkIntroAsSeen2()
{
	// 2번째 오프닝을 봤다고 체크!
	PermData.bHasSeenIntro2 = true;

	// 이 중요한 사실을 잊어버리지 않게 즉시 디스크(영구 세이브)에 덮어씁니다.
	SavePermToDisk();

	UE_LOG(LogTemp, Warning, TEXT("[SaveSystem] 플레이어가 최초 오프닝을 시청했습니다. 영구 저장 완료!"));
}

bool USPSaveGameSubsystem::HasSeenIntro() const
{
	return PermData.bHasSeenIntro;
}

bool USPSaveGameSubsystem::HasSeenIntro2() const
{
	return PermData.bHasSeenIntro2;
}

void USPSaveGameSubsystem::SavePermToDisk()
{
	USPPermSaveGame* SaveInst = Cast<USPPermSaveGame>(UGameplayStatics::CreateSaveGameObject(USPPermSaveGame::StaticClass()));
	if (SaveInst)
	{
		SaveInst->PermData = PermData;
		UGameplayStatics::SaveGameToSlot(SaveInst, PermSlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("영구 데이터 디스크 저장 완료! [%s]"), *PermSlotName);
	}
}

bool USPSaveGameSubsystem::LoadPermFromDisk()
{
	if (UGameplayStatics::DoesSaveGameExist(PermSlotName, 0))
	{
		USPPermSaveGame* LoadInst = Cast<USPPermSaveGame>(UGameplayStatics::LoadGameFromSlot(PermSlotName, 0));
		if (LoadInst)
		{
			PermData = LoadInst->PermData;
			UE_LOG(LogTemp, Warning, TEXT("영구 데이터 디스크 로드 성공!"));
			return true;
		}
	}
	return false;
}