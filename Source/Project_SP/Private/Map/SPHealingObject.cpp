// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/SPHealingObject.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Component/InventoryComponent.h"
#include "Map/MapBase.h"
#include "Map/MapManagerSubSystem.h"
#include "Data/RewardDataStructs.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"

ASPHealingObject::ASPHealingObject()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void ASPHealingObject::ExecuteInteraction(AActor* Interactor)
{
	if (!Interactor) return;

	APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(Interactor)->GetController());
	UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
	UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Interactor);

	if (!PC || !MapManager || !InventoryComp || !ASC) return;

	IInteractableInterface::Execute_PlayInteractSound(this);


	float MaxHealth = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxHealthAttribute());
	float CurrentHealth = ASC->GetNumericAttribute(USPGASAttributeSet::GetHealthAttribute());
	float HealAmount = MaxHealth * 0.5f;
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetHealthAttribute(), FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth));

	float MaxTP = ASC->GetNumericAttribute(USPGASAttributeSet::GetMaxTimePowerAttribute());
	float CurrentTP = ASC->GetNumericAttribute(USPGASAttributeSet::GetTimePowerAttribute());
	float TPHealAmount = MaxTP * 0.1f;
	ASC->SetNumericAttributeBase(USPGASAttributeSet::GetTimePowerAttribute(), FMath::Clamp(CurrentTP + TPHealAmount, 0.0f, MaxTP));

	UE_LOG(LogTemp, Log, TEXT("[회복 오브젝트] 체력을 50%% 회복했습니다! (현재: %f)"), CurrentHealth + HealAmount);
	UE_LOG(LogTemp, Log, TEXT("[회복 오브젝트] TP를 10%% 회복했습니다! (현재: %f)"), CurrentTP + TPHealAmount);

	// ==========================================
	// 2. 가중치 보상 추첨 돌리기! (bIsHealingObject = true)
	// ==========================================
	int32 CurrentStage = MapManager->GetCurrentStage();
	EMapType CurrentMapType = MapManager->GetCurrentMapType();

	FRewardResult Reward = MapManager->GenerateInteractableReward(true, CurrentStage, CurrentMapType);

	// ==========================================
	// 3. 일반 재화 즉시 지급 (인벤토리)
	// ==========================================
	if (MapManager)
	{
		MapManager->PendingToastRewards.Empty();
		MapManager->PendingExpReward = 0;

		if (Reward.Gold > 0) MapManager->PendingToastRewards.FindOrAdd(EResourceType::Gold) += Reward.Gold;
		if (Reward.Sand > 0) MapManager->PendingToastRewards.FindOrAdd(EResourceType::Sand) += Reward.Sand;
		if (Reward.IncompleteEnergy > 0) MapManager->PendingToastRewards.FindOrAdd(EResourceType::IncompleteEnergy) += Reward.IncompleteEnergy;
		if (Reward.Fragment > 0) MapManager->PendingToastRewards.FindOrAdd(EResourceType::Fragment) += Reward.Fragment;
		if (Reward.Exp > 0) MapManager->PendingExpReward += Reward.Exp;
	}

	//  그 다음 인벤토리 지급
	if (Reward.Gold > 0) InventoryComp->AddMoney(Reward.Gold);
	if (Reward.Sand > 0) InventoryComp->AddSand(Reward.Sand);
	if (Reward.IncompleteEnergy > 0) InventoryComp->AddIncompleteEnergy(Reward.IncompleteEnergy);
	if (Reward.Fragment > 0) InventoryComp->AddFragment(Reward.Fragment);

	InventoryComp->OnOpenGetInventory.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("포션에서 재화 지급"));

	if (Reward.Exp > 0)
	{
		float CurrentExp = ASC->GetNumericAttribute(USPGASAttributeSet::GetExperienceAttribute());
		ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), CurrentExp + Reward.Exp);
	}

	// ==========================================
	// 4. 대박! 유물이 당첨되었다면? (UI 띄우기)
	// ==========================================
	if (Reward.RelicRewardCount > 0 && RelicRewardWidgetClass)
	{
		UUserWidget* RewardUI = CreateWidget<UUserWidget>(PC, RelicRewardWidgetClass);
		if (RewardUI)
		{
			// 추첨 횟수 전달
			if (FProperty* CountProp = RewardUI->GetClass()->FindPropertyByName(FName("RelicRewardCount")))
			{
				if (FIntProperty* IntProp = CastField<FIntProperty>(CountProp))
				{
					IntProp->SetPropertyValue_InContainer(RewardUI, Reward.RelicRewardCount);
				}
			}

			// 보스 여부 전달 (회복 오브젝트는 10% 확률 대박이므로 기본 확률을 타도록 보스 판정은 그대로 둡니다)
			if (FProperty* BossProp = RewardUI->GetClass()->FindPropertyByName(FName("bIsBossReward")))
			{
				if (FBoolProperty* BoolProp = CastField<FBoolProperty>(BossProp))
				{
					BoolProp->SetPropertyValue_InContainer(RewardUI, Reward.bIsBossReward);
				}
			}

			RewardUI->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(RewardUI->GetCachedWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;

			UE_LOG(LogTemp, Warning, TEXT("[회복 오브젝트] 10%% 확률 대박! 유물 보상 창이 열립니다."));
		}
	}

	// ==========================================
	// 5. 맵 클리어 처리 및 포탈 활성화
	// ==========================================
	AActor* FoundMap = UGameplayStatics::GetActorOfClass(GetWorld(), AMapBase::StaticClass());
	if (AMapBase* CurrentMap = Cast<AMapBase>(FoundMap))
	{
		CurrentMap->SetMapState(EMapState::Cleared);
		UE_LOG(LogTemp, Warning, TEXT("[회복 맵] 상호작용 완료! 포탈이 활성화됩니다."));
	}

	// ==========================================
	// 6. 상호작용 완료 (사운드 및 파괴)
	// ==========================================
	Destroy();
}

FText ASPHealingObject::GetInteractText() const
{
	return FText::FromString(TEXT("회복 하기[F]"));
}

void ASPHealingObject::PlayInteractSound_Implementation()
{
}

