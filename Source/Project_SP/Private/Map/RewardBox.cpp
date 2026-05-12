#include "Map/RewardBox.h"
#include "Map/MapBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

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
	if (bIsOpened) return;

	// 상호작용한 Actor가 플레이어인지 확인
	APlayerController* PC = Cast<APlayerController>(Cast<ACharacter>(Interactor)->GetController());
	if (!PC) return;

	bIsOpened = true;

	//보상 지급 코드를 여기서 구현
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. 유물 보상 위젯 생성
	UUserWidget* RewardUI = CreateWidget<UUserWidget>(PC, RelicRewardWidgetClass);
	if (RewardUI)
	{
		// 2. 위젯에 현재 스테이지 값 전달 (WBP_RelicReward의 CurrentStage 변수 이름과 일치해야 함)
		FProperty* StageProp = RewardUI->GetClass()->FindPropertyByName(FName("CurrentStage"));
		if (StageProp)
		{
			if (FIntProperty* IntProp = CastField<FIntProperty>(StageProp))
			{
				IntProp->SetPropertyValue_InContainer(RewardUI, StageLevel);
			}
		}

		// 3. 화면에 표시 및 입력 모드 설정
		RewardUI->AddToViewport();

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(RewardUI->GetCachedWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
