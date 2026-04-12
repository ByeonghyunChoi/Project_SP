// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/SPUIDataSubsystem.h"
#include "Data/SPActiveEffectUIDictionary.h"

void USPUIDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString AssetPath = TEXT("/Game/DataTable/DataAsset/BuffDataAsset/DA_GlobalUIDictionary.DA_GlobalUIDictionary");
	GlobalUIDictionary = LoadObject<USPActiveEffectUIDictionary>(nullptr, *AssetPath);
}

bool USPUIDataSubsystem::GetEffectUIData(FGameplayTag EffectTag, FEffectUIData& OutData)
{
	if (GlobalUIDictionary)
	{
		return GlobalUIDictionary->GetEffectUIData(EffectTag, OutData);
	}
	return false;
}
