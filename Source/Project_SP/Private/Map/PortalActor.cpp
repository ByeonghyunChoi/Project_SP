#include "Map/PortalActor.h"
#include "Map/MapManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

APortalActor::APortalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	SetRootComponent(MeshComponent);
}

void APortalActor::ExecuteInteraction(AActor* Interactor)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UMapManagerSubsystem* MapManager = GI->GetSubsystem<UMapManagerSubsystem>();
	if (MapManager)
	{
		// 매니저에게 내가 가진 목적지 타입을 전달하며 이동 요청
		MapManager->MoveToNextFloor(TargetMapType);
	}
}

FText APortalActor::GetInteractText() const
{
	// Enum에서 순수 이름 문자열만 추출 (예: "NormalBattle")
	FString EnumName = StaticEnum<EMapType>()->GetNameStringByValue((int64)TargetMapType);

	// 나중에 데이터 테이블에서 한글 이름을 가져오도록 확장하기 좋습니다.
	return FText::Format(NSLOCTEXT("Portal", "MoveFormat", "{0} (으)로 이동"), FText::FromString(EnumName));
}