// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/OpartsBase.h"
#include "Component/InventoryComponent.h"

//생성자
UOpartsBase::UOpartsBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentLevel = 1; // 초기 레벨 설정
}



void UOpartsBase::BeginPlay()
{
	Super::BeginPlay();
	// InventoryComponent 참조 찾기
	if (GetOwner())
	{
		inventoryRef = GetOwner()->FindComponentByClass<UInventoryComponent>();
		if (!inventoryRef)
		{
			UE_LOG(LogTemp, Error, TEXT("InventoryComponent not found on the Owner Actor! Check setup."));
		}
	}

	// 초기 스탯 설정 (레벨 1의 스탯)
	// LevelStats는 인덱스 0부터 시작하므로, 레벨 1의 스탯은 인덱스 1에 있을 것으로 가정
	if (LevelStats.IsValidIndex(CurrentLevel)) // CurrentLevel은 현재 1
	{
		CurrentStats = LevelStats[CurrentLevel];

		UE_LOG(LogTemp, Log, TEXT("Oparts Initialized. Level: %d, Health: %.1f, Attack: %.1f"),
			CurrentLevel, CurrentStats.Health, CurrentStats.Attack);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LevelStats array does not have stats for initial level %d! Check data table setup."), CurrentLevel);
	}
}


void UOpartsBase::LevelUpOparts()
{
	// 1. 레벨 제한 체크
	if (CurrentLevel >= 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("Oparts LevelUp failed: Max Level Reached. CurrentLevel: %d"), CurrentLevel);
		return;
	}

	// 2. InventoryComponent 참조 체크
	if (!inventoryRef)
	{
		UE_LOG(LogTemp, Error, TEXT("Oparts LevelUp failed: InventoryComponent is NULL. Cannot check/spend Sand."));
		return;
	}

	// 3. 필요 모래 요구량 체크
	int32 Required = GetRequiredSandForNextLevel();

	// 요구치가 0보다 작으면
	if (Required < 0)
	{
		// GetRequiredSandForNextLevel()에서 -1 반환은 데이터 오류나 최대 레벨 도달을 의미
		UE_LOG(LogTemp, Error, TEXT("Oparts LevelUp failed: Required Sand data not found for next level (%d -> %d)."), CurrentLevel, CurrentLevel + 1);
		return;
	}

	// 현재 모래 소유량 가져오기
	int32 CurrentSand = inventoryRef->GetCurrentSand();

	// 4. 모래 소유량 체크 - 부족 시 레벨업 실패
	if (CurrentSand < Required)
	{
		UE_LOG(LogTemp, Warning, TEXT("Oparts LevelUp failed: Not enough Sand. Current Sand: %d, Required: %d"), CurrentSand, Required);
		return;
	}

	// 5. 레벨업 진행

	// 모래 소모
	int32 NewSandAmount = CurrentSand - Required;
	inventoryRef->SetCurrentSand(NewSandAmount);

	// 레벨 증가
	CurrentLevel++;

	// 6. 새 스탯 적용
	// CurrentLevel은 이제 다음 레벨을 가리킴
	if (LevelStats.IsValidIndex(CurrentLevel))
	{
		// 레벨스탯 배열에서 현재 레벨에 해당하는 스탯을 가져와 적용
		CurrentStats = LevelStats[CurrentLevel];

		UE_LOG(LogTemp, Log, TEXT("Oparts Level Up Success! New Level: %d, Health: %.1f, Attack: %.1f"),
			CurrentLevel, CurrentStats.Health, CurrentStats.Attack);
	}
	else
	{
		// 데이터 세팅 오류
		UE_LOG(LogTemp, Error, TEXT("Oparts Level Up Success, but failed to apply stats! LevelStats array does not have stats for level %d."), CurrentLevel);
	}
}

// 특수 능력 함수(나중에 같이 구현)
void UOpartsBase::ActiveSpecialAbility()
{

}

int32 UOpartsBase::GetOpartsCurrentLevel() const
{
	return CurrentLevel; // 오파츠에 띄우기 위한 반환값
}

int32 UOpartsBase::GetRequiredSandForNextLevel() const
{
	if (RequiredSand.IsValidIndex(CurrentLevel))
	{
		// RequiredSand 배열은 [Lv0 -> Lv1], [Lv1 -> Lv2] ... 레벨업에 필요한 모래 양을 담고 있어야 합니다.
		return RequiredSand[CurrentLevel];
	}
	// 다음 레벨에 필요한 모래 데이터가 없는 경우 (최대 레벨 도달 혹은 데이터 오류)
	return -1;
}

