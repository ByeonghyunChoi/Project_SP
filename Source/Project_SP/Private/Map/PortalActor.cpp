#include "Map/PortalActor.h"
#include "Map/MapManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

APortalActor::APortalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	SetRootComponent(MeshComponent);

	PortalVisualComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("PortalVisual"));
	PortalVisualComponent->SetupAttachment(RootComponent);
}

void APortalActor::ExecuteInteraction(AActor* Interactor)
{
	IInteractableInterface::Execute_PlayInteractSound(this);
}

FText APortalActor::GetInteractText() const
{
	// Enum에서 순수 이름 문자열만 추출 (예: "NormalBattle")
	FString EnumName = StaticEnum<EMapType>()->GetNameStringByValue((int64)TargetMapType);

	// 나중에 데이터 테이블에서 한글 이름을 가져오도록 확장하기 좋습니다.
	return FText::Format(NSLOCTEXT("Portal", "MoveFormat", "{0} (으)로 이동"), FText::FromString(EnumName));
}

bool APortalActor::CanInteract(AActor* Interactor) const
{
	return bIsActive;
}

void APortalActor::PlayInteractSound_Implementation()
{
}

void APortalActor::SetPortalTargetType(EMapType InType)
{
	TargetMapType = InType;

	if (PortalVisualClasses.Contains(TargetMapType))
	{
		// 바구니에 블루프린트 클래스를 설정하면, 자동으로 해당 액터가 스폰되어 자식으로 들어옵니다.
		PortalVisualComponent->SetChildActorClass(PortalVisualClasses[TargetMapType]);

		UE_LOG(LogTemp, Log, TEXT("포탈 외형 교체 완료: %d"), (int32)TargetMapType);
	}

	// 타겟 타입이 정해지는 즉시 블루프린트에 알림
	OnUpdatePortalColor(TargetMapType);
}

void APortalActor::ActivatePortal(bool bActive)
{
	bIsActive = bActive;

	// 시각적 효과 (블루프린트에서 파티클 켜기/끄기 구현)
	OnPortalStateChanged(bActive);
}

void APortalActor::ExecutePortalTransition()
{
	// 맵 이동 핵심 로직
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	if (!bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal is not active yet!"));
		return;
	}

	UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>();
	if (MapManager)
	{
		bIsActive = false;
		// 매니저에게 내가 가진 목적지 타입을 전달하며 이동 요청
		MapManager->MoveToNextFloor(TargetMapType);
	}
}
