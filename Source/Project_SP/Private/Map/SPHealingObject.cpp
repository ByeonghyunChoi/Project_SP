// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/SPHealingObject.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Map/MapBase.h"

ASPHealingObject::ASPHealingObject()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void ASPHealingObject::ExecuteInteraction(AActor* Interactor)
{
	if (!Interactor) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Interactor);
	if (ASC)
	{
		float MaxHealth = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), MaxHealth);
		IInteractableInterface::Execute_PlayInteractSound(this);

		UE_LOG(LogTemp, Log, TEXT("[회복]"));
	}

	AActor* FoundMap = UGameplayStatics::GetActorOfClass(GetWorld(), AMapBase::StaticClass());
	AMapBase* CurrentMap = Cast<AMapBase>(FoundMap);

	if (CurrentMap)
	{
		CurrentMap->SetMapState(EMapState::Cleared);
		UE_LOG(LogTemp, Warning, TEXT("[회복 맵] 상호작용 완료! 포탈이 활성화됩니다."));
	}

	Destroy();
}

FText ASPHealingObject::GetInteractText() const
{
	return FText::FromString(TEXT("회복 하기[F]"));
}

void ASPHealingObject::PlayInteractSound_Implementation()
{
}

