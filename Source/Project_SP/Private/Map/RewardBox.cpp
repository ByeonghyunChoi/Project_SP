#include "Map/RewardBox.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Map/MapManagerSubSystem.h"
#include "Data/RewardDataStructs.h"
#include "Data/SPDataStructs.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


ARewardBox::ARewardBox()
{
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetHiddenInGame(true);

	RewardParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RewardParticle"));
	RewardParticle->SetupAttachment(RootComponent);
}

void ARewardBox::ExecuteInteraction(AActor* Interactor)
{
	// ==========================================
	// 1. 방어 코드 및 중복 실행 방지
	// ==========================================
	if (bIsOpened || !Interactor) return;

	APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(Interactor)->GetController());
	if (!PC) return;

	bIsOpened = true; // 열림 상태 확정 (광클 방지)

	// ==========================================
	// 2. 매니저 및 컴포넌트 가져오기
	// ==========================================
	UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
	UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();

	if (!MapManager || !InventoryComp) return;

	// ==========================================
	// 3. 보상 추첨기 돌리기! (가중치 룰 적용)
	// ==========================================
	int32 CurrentStage = MapManager->GetCurrentStage();
	EMapType CurrentMapType = MapManager->GetCurrentMapType();

	// 상자(false) 보상 추첨!
	FRewardResult Reward = MapManager->GenerateInteractableReward(false, CurrentStage, CurrentMapType);

	// ==========================================
	// 4. 일반 재화 즉시 지급 (인벤토리)
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

		UE_LOG(LogTemp, Warning, TEXT("대기열 추가 완료"));
	}

	//  2. 대기열 세팅이 끝났으니 실제 재화를 지급합니다! (이때 InventoryComp 안에서 방송이 나가고, UI가 방금 넣은 큐를 읽어옵니다)
	if (Reward.Gold > 0) InventoryComp->AddMoney(Reward.Gold);
	if (Reward.Sand > 0) InventoryComp->AddSand(Reward.Sand);
	if (Reward.IncompleteEnergy > 0) InventoryComp->AddIncompleteEnergy(Reward.IncompleteEnergy);
	if (Reward.Fragment > 0) InventoryComp->AddFragment(Reward.Fragment);

	InventoryComp->OnOpenGetInventory.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("상자 보상 지급 완료"));

	// (경험치 보상이 있다면 ASC를 통해 지급)
	if (Reward.Exp > 0)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Interactor);
		if (ASC)
		{
			float CurrentExp = ASC->GetNumericAttribute(USPGASAttributeSet::GetExperienceAttribute());
			ASC->SetNumericAttributeBase(USPGASAttributeSet::GetExperienceAttribute(), CurrentExp + Reward.Exp);
		}
	}

	// ==========================================
	// 5. 유물이 당첨되었다면? (UI 띄우기) / 아니라면 즉시 클리어!
	// ==========================================
	if (Reward.RelicRewardCount > 0 && RelicRewardWidgetClass)
	{
		UUserWidget* RewardUI = CreateWidget<UUserWidget>(PC, RelicRewardWidgetClass);
		if (RewardUI)
		{
			if (FProperty* CountProp = RewardUI->GetClass()->FindPropertyByName(FName("RelicRewardCount")))
			{
				if (FIntProperty* IntProp = CastField<FIntProperty>(CountProp))
				{
					IntProp->SetPropertyValue_InContainer(RewardUI, Reward.RelicRewardCount);
				}
			}

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
		}
	}
	else
	{
		// 🌟 [추가된 로직] 유물이 당첨되지 않았을 경우, UI가 없으므로 여기서 바로 맵을 클리어 처리합니다!
		AActor* FoundMap = UGameplayStatics::GetActorOfClass(GetWorld(), AMapBase::StaticClass());
		if (AMapBase* CurrentMap = Cast<AMapBase>(FoundMap))
		{
			CurrentMap->SetMapState(EMapState::Cleared);
			UE_LOG(LogTemp, Warning, TEXT("[보상 상자] 유물이 당첨되지 않아 즉시 맵을 클리어 처리합니다. 포탈 활성화!"));
		}
	}

	// ==========================================
	// 6. 상호작용 완료 (사운드 및 상자 파괴)
	// ==========================================
	IInteractableInterface::Execute_PlayInteractSound(this);
	Destroy();
}

FText ARewardBox::GetInteractText() const
{
	return FText::FromString(TEXT("보상 열기"));
}

void ARewardBox::PlayInteractSound_Implementation()
{
}

void ARewardBox::SetupParticleByMapType(EMapType InMapType)
{
	if (!RewardParticle) return; // 파티클 컴포넌트가 없으면 종료

	UNiagaraSystem* SelectedParticle = nullptr;

	// 1. 맵 타입에 따라 켤 파티클 결정
	switch (InMapType)
	{
	case EMapType::NormalBattle: // (선생님의 Enum 이름에 맞게 수정해주세요)
		SelectedParticle = NormalParticle;
		break;
	case EMapType::Jester:
	case EMapType::StrongEnemyBattle:
		SelectedParticle = EpicParticle;
		break;
	case EMapType::BossBattle:
		SelectedParticle = BossParticle;
		break;
	default:
		SelectedParticle = NormalParticle;
		break;
	}

	// 2. 파티클 갈아끼우고 켜기!
	if (SelectedParticle)
	{
		RewardParticle->SetAsset(SelectedParticle);
		RewardParticle->Activate(true); // 재생 버튼 누르기!
	}
}
