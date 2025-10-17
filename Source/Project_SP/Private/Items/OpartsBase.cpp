
#include "Component/OpartsComponent.h"

// Sets default values for this component's properties
UOpartsComponent::UOpartsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentLevel = 1; // 초기 레벨 설정
	
}

void UOpartsComponent::LevelUpOparts()
{
	inventoryRef = NewObject<UInventoryComponent>();

	// 1. 레벨 제한 체크
	if (CurrentLevel >= 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("Oparts LevelUp failed: Max Level Reached. CurrentLevel: %d"), CurrentLevel);
		return;
	}

	//필요 모래
	int32 Required = GetRequiredSandForNextLevel();
	inventoryRef->getCurrentSand()

	//레벨업
	CurrentLevel++;

	if (LevelStats.IsValidIndex(CurrentLevel))
	{
		CurrentStats = LevelStats[CurrentLevel];
	}

	UE_LOG(LogTemp, Log, TEXT("Oparts Level Up Success! New Level: %d, Health: %.1f, Attack: %.1f"),
		CurrentLevel, CurrentStats.Health, CurrentStats.Attack);
}

void UOpartsComponent::ActiveSpecialAbility()
{
}

int32 UOpartsComponent::GetOpartsCurrentLevel() const
{
	return CurrentLevel; // 오파츠에 띄우기 위한 반환값
}

int32 UOpartsComponent::GetRequiredSandForNextLevel() const
{
	if (RequiredSand.IsValidIndex(CurrentLevel))
	{
		// RequiredSand 배열은 [Lv0 -> Lv1], [Lv1 -> Lv2] ... 레벨업에 필요한 모래 양을 담고 있어야 합니다.
		return RequiredSand[CurrentLevel];
	}
	return -1;
}




