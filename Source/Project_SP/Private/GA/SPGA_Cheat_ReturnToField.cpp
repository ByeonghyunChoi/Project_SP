// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/SPGA_Cheat_ReturnToField.h"
#include "Map/MapManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

USPGA_Cheat_ReturnToField::USPGA_Cheat_ReturnToField()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

void USPGA_Cheat_ReturnToField::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("?? [Cheat] Returning to Field..."));

	// MapManager를 찾아서 복귀 함수 호출
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (UMapManagerSubsystem* MapManager = GI ? GI->GetSubsystem<UMapManagerSubsystem>() : nullptr)
	{
		MapManager->ReturnToField();
	}

	// 어빌리티 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
