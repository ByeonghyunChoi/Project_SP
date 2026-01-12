// Fill out your copyright notice in the Description page of Project Settings.


//#include "Items/OpartsBase.h"
//#include "Component/InventoryComponent.h"
//
////생성자
//UOpartsBase::UOpartsBase()
//{
//	PrimaryComponentTick.bCanEverTick = false;
//	CurrentLevel = 1; // 초기 레벨 설정
//
//	// 레벨업에 필요한 모래 양 설정 0레벨은 필요없으므로 0으로 둠
//	RequiredSand = { 0, 100, 150, 250, 400};
//	// 다음 해금에 필요한 불완전한 기운 설정
//	RequiredIncompleteEnergy = { 1, 2, 3, 3, 4 };
//	// 아티팩트 해금 상태 초기화 (최대 5개)
//	//bIsArtifactUnlocked.Init(false, 5);
//	ArtifactUnlockedNumber = 0;
//	bIsArtifactUnlocked = { false, false, false, false, false };
//}

//데이터 테이블에서 스탯을 가져오는 헬퍼 함수 구현
//bool UOpartsBase::GetStatsForLevel(int32 Level, FOpartStats& OutStats)
//{
//	if (!OpartsStatsDataTable)
//	{
//		UE_LOG(LogTemp, Error, TEXT("OpartsStatsDataTable is null! Cannot load stats."));
//		return false;
//	}
//
//	// 데이터 테이블 RowName은 "LEVEL_X" 형태로 저장한다고 가정합니다.
//	FString RowName = FString::Printf(TEXT("LEVEL_%d"), Level);
//
//	// 데이터 테이블에서 해당 Row를 찾습니다.
//	FOpartsStatRow* StatsRow = OpartsStatsDataTable->FindRow<FOpartsStatRow>(FName(*RowName), TEXT(""));
//
//	if (StatsRow)
//	{
//		// 찾은 데이터를 CurrentStats 구조체에 복사
//		OutStats.Health = StatsRow->Health;
//		OutStats.Attack = StatsRow->Attack;
//		OutStats.Speed = StatsRow->Speed;
//		return true;
//	}
//
//	UE_LOG(LogTemp, Error, TEXT("Failed to find stats for Level %d (Row: %s) in DataTable."), Level, *RowName);
//	return false;
//}

//bool UOpartsBase::GetStatsForNextLevel(int32 Level, FOpartStats& OutStats) const
//{
//	if (!OpartsStatsDataTable)
//	{
//		UE_LOG(LogTemp, Error, TEXT("OpartsStatsDataTable is null! Cannot load stats."));
//		return false;
//	}
//	FString RowName = FString::Printf(TEXT("LEVEL_%d"), Level);
//
//	FOpartsStatRow* StatsRow = OpartsStatsDataTable->FindRow<FOpartsStatRow>(FName(*RowName), TEXT(""));
//    
//    // ...
//    // 데이터 로드 성공 시:
//    if (StatsRow)
//    {
//        // 찾은 데이터를 OutStats 구조체에 복사
//        OutStats.Health = StatsRow->Health;
//        OutStats.Attack = StatsRow->Attack;
//        OutStats.Speed = StatsRow->Speed;
//        return true;
//    }
//
//	UE_LOG(LogTemp, Error, TEXT("Failed to find stats for Level %d (Row: %s) in DataTable."), Level, *RowName);
//	return false; //
//}

//void UOpartsBase::BeginPlay()
//{
//	Super::BeginPlay();
//	// InventoryComponent 참조 찾기
//	if (GetOwner())
//	{
//		inventoryRef = GetOwner()->FindComponentByClass<UInventoryComponent>();
//		if (!inventoryRef)
//		{
//			UE_LOG(LogTemp, Error, TEXT("InventoryComponent not found on the Owner Actor! Check setup."));
//		}
//	}
//
//	 //초기 스탯 설정 (레벨 1의 스탯)
//	 //LevelStats는 인덱스 0부터 시작하므로, 레벨 1의 스탯은 인덱스 1에 있을 것으로 가정
//	if (GetStatsForLevel(CurrentLevel, CurrentStats))
//	{
//		UE_LOG(LogTemp, Log, TEXT("Oparts Initialized. Level: %d, Health: %.1f, Attack: %.1f"),
//			CurrentLevel, CurrentStats.Health, CurrentStats.Attack);
//	}
//}
//
//
//void UOpartsBase::LevelUpOparts()
//{
//	// 1. 레벨 제한 체크
//	if (CurrentLevel >= 5)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Oparts LevelUp failed: Max Level Reached. CurrentLevel: %d"), CurrentLevel);
//		return;
//	}
//
//	// 2. InventoryComponent 참조 체크
//	if (!inventoryRef)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Oparts LevelUp failed: InventoryComponent is NULL. Cannot check/spend Sand."));
//		return;
//	}
//
//	// 3. 필요 모래 요구량 체크
//	int32 Required = GetRequiredSandForNextLevel();
//
//	// 요구치가 0보다 작으면
//	if (Required < 0)
//	{
//		// GetRequiredSandForNextLevel()에서 -1 반환은 데이터 오류나 최대 레벨 도달을 의미
//		UE_LOG(LogTemp, Error, TEXT("Oparts LevelUp failed: Required Sand data not found for next level (%d -> %d)."), CurrentLevel, CurrentLevel + 1);
//		return;
//	}
//
//	// 현재 모래 소유량 가져오기
//	int32 CurrentSand = inventoryRef->GetCurrentSand();
//
//	// 4. 모래 소유량 체크 - 부족 시 레벨업 실패
//	if (CurrentSand < Required)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Oparts LevelUp failed: Not enough Sand. Current Sand: %d, Required: %d"), CurrentSand, Required);
//		return;
//	}
//
//	// 5. 레벨업 진행
//
//	// 모래 소모
//	int32 NewSandAmount = CurrentSand - Required;
//	inventoryRef->SetCurrentSand(NewSandAmount);
//
//	// 레벨 증가
//	CurrentLevel++;
//
//	// 6. 새 스탯 적용
//	if (GetStatsForLevel(CurrentLevel, CurrentStats))
//	{
//		GetCalculatedModifiers();
//
//		UE_LOG(LogTemp, Log, TEXT("Oparts Level Up Success! New Level: %d, Health: %.1f, Attack: %.1f"),CurrentLevel, CurrentStats.Health, CurrentStats.Attack);
//	}
//	else
//	{
//		// 데이터 세팅 오류
//		UE_LOG(LogTemp, Error, TEXT("Oparts Level Up Success, but failed to apply stats! LevelStats array does not have stats for level %d."), CurrentLevel);
//	}
//
//	CalculateMaterial.Broadcast();
//}
//
//// 아티팩트 해금 함수 (UI에 적용할때 Unlock함수를 실행 버튼에 옮겨놓고 눌러서 실행이 되면 브로드캐스트 알림을 받고 알림을 받으면 UI에 해금된 아티팩트 표시 오파츠 마다 개인 설정 필요)
//void UOpartsBase::UnlockArtifact()
//{
//	// 1. 최대 해금 횟수 체크 (5개 해금 완료 시)
//	if (ArtifactUnlockedNumber >= bIsArtifactUnlocked.Num())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Artifact UnLock failed: Max Artifact Slots already unlocked. Count: %d"), ArtifactUnlockedNumber);
//		return;
//	}
//
//	//InventoryComponent 참조 체크
//	if (!inventoryRef)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Artifact Unlock failed: InventoryComponent is NULL. Cannot check/spend Sand."));
//		return;
//	}
//
//	// 3. 필요 불완전한 기운 요구량 체크 (현재 해금 횟수를 인덱스로 사용)
//	int32 RequiredEnergy = GetRequiredIncompleteEnergy();
//	// 요구치가 0보다 작으면
//	if (RequiredEnergy < 0)
//	{
//		// GetRequiredIncompleteEnergy()에서 -1 반환은 데이터 오류를 의미
//		UE_LOG(LogTemp, Error, TEXT("Artifact UnLock failed: Required Energy data not found for slot %d."), ArtifactUnlockedNumber + 1);
//		return;
//	}
//
//	// 현재 기운 소유량 체크
//	int32 CurrentIncompleteEnergy = inventoryRef->GetCurrentIncompleteEnergy();
//
//	// 4. 재화 소유량 체크 - 부족 시 해금 실패
//	if (CurrentIncompleteEnergy < RequiredEnergy)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Artifact Unlock failed: Not enough Energy. Current Energy: %d, Required: %d"), CurrentIncompleteEnergy, RequiredEnergy);
//		return;
//	}
//	// 재화 소모
//	int32 NewEnergyAmount = CurrentIncompleteEnergy - RequiredEnergy;
//	inventoryRef->SetCurrentIncompleteEnergy(NewEnergyAmount);
//
//	// 아티팩트 슬롯 해금 상태 기록 (현재 ArtifactUnlockedNumber 인덱스)
//	if (bIsArtifactUnlocked.IsValidIndex(ArtifactUnlockedNumber))
//	{
//		bIsArtifactUnlocked[ArtifactUnlockedNumber] = true;
//	}
//
//	ArtifactUnlockedNumber++;
//
//	CalculateMaterial.Broadcast();
//}
//
//// 특수 능력 함수(나중에 같이 구현)
//void UOpartsBase::ActiveSpecialAbility()
//{
//	
//}
//
//int32 UOpartsBase::GetOpartsCurrentLevel() const
//{
//	return CurrentLevel; // 오파츠에 띄우기 위한 반환값
//}
//
//bool UOpartsBase::GetNextLevelStats(FOpartStats& OutNextStats) const
//{
//	const int32 MAX_OPARTS_LEVEL = 5;
//
//	if (CurrentLevel >= MAX_OPARTS_LEVEL)
//	{
//		// 다음 레벨이 없음을 알림
//		OutNextStats = FOpartStats();
//		return false;
//	}
//
//	// 2. 다음 레벨(CurrentLevel + 1) 계산
//	int32 NextLevel = CurrentLevel + 1;
//
//	// 3. 헬퍼 함수를 사용하여 다음 레벨의 데이터를 로드
//	if (GetStatsForNextLevel(NextLevel, OutNextStats))
//	{
//		return true; // 로드 성공
//	}
//
//	// 데이터 테이블에 다음 레벨 스탯이 없는 경우
//	OutNextStats = FOpartStats();
//	return false;
//}
//
//
//int32 UOpartsBase::GetRequiredSandForNextLevel() const
//{
//	if (RequiredSand.IsValidIndex(CurrentLevel))
//	{
//		// RequiredSand 배열은 [Lv0 -> Lv1], [Lv1 -> Lv2] ... 레벨업에 필요한 모래 양을 담고 있어야 합니다.
//		return RequiredSand[CurrentLevel];
//	}
//	// 다음 레벨에 필요한 모래 데이터가 없는 경우 (최대 레벨 도달 혹은 데이터 오류)
//	return -1;
//}
//
//int32 UOpartsBase::GetRequiredIncompleteEnergy() const
//{
//	// ArtifactUnlockedNumber가 배열의 유효한 인덱스인지 확인
//	if (RequiredIncompleteEnergy.IsValidIndex(ArtifactUnlockedNumber))
//	{
//		// ArtifactUnlockedNumber (0부터 시작)를 인덱스로 사용
//		return RequiredIncompleteEnergy[ArtifactUnlockedNumber];
//	}
//	// 해금 데이터가 없거나 모두 해금한 경우
//	return -1;
//}

//FStatModifiers UOpartsBase::GetCalculatedModifiers() const
//{
//	FStatModifiers Result;
//
//	if (LevelStatTable)
//	{
//		// 행 이름 생성 (예: "Level_1", "Level_2")
//		// 스크린샷에 있는 행 이름과 똑같은 형식을 맞춰야 합니다.
//		FName RowName = *FString::Printf(TEXT("Level_%d"), CurrentLevel);
//
//		// 테이블에서 행 찾기
//		static const FString ContextString(TEXT("Oparts Stat Lookup"));
//		FOpartsStatRow* RowData = LevelStatTable->FindRow<FOpartsStatRow>(RowName, ContextString);
//
//		if (RowData)
//		{
//			// 찾았으면 결과에 넣어줍니다.
//			Result.FlatHP = RowData->Health;
//			Result.FlatAttack = RowData->Attack;
//			Result.FlatSpeed = RowData->Speed;
//		}
//		else
//		{
//			UE_LOG(LogTemp, Warning, TEXT("오파츠 데이터 테이블에서 %s 행을 찾을 수 없습니다!"), *RowName.ToString());
//		}
//	}
//
//	return Result;
//}
//
////  장착 시 호출 (스탯 적용 등)
//void UOpartsBase::OnEquip(AActor* Instigator)
//{
//	// AttributesComponent 참조 찾기
//	UAttributesComponent* AttributesComp = Instigator->FindComponentByClass<UAttributesComponent>();
//
//	if (AttributesComp && !bStatsCurrentlyApplied)
//	{
//		// 오파츠의 현재 스탯 (CurrentStats)을 AttributesComponent에 적용 요청
//		//AttributesComp->ApplyOpartsStats(CurrentStats);
//		bStatsCurrentlyApplied = true;
//	}
//}
//
//// 해제 시 호출 (스탯 제거 등)
//void UOpartsBase::OnUnequip(AActor* Instigator)
//{
//	// AttributesComponent 참조 찾기
//	UAttributesComponent* AttributesComp = Instigator->FindComponentByClass<UAttributesComponent>();
//
//	// 스탯이 현재 적용된 상태일 때만 제거 로직 실행
//	if (AttributesComp && bStatsCurrentlyApplied)
//	{
//		//AttributesComp->RemoveOpartsStats(CurrentStats);
//		bStatsCurrentlyApplied = false; // 스탯 제거 완료
//	}
//}
