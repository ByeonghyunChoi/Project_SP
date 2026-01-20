// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/PortalActor.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Map/MapNode.h"
#include "Map/MapManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Character/SPGASPlayerCharacter.h"

// Sets default values
APortalActor::APortalActor()
{
	PortalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
	SetRootComponent(PortalRoot);

	OverlapVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapVolume"));
	OverlapVolume->SetupAttachment(RootComponent);
	OverlapVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnOverlapBegin);

	InfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidget"));
	InfoWidget->SetupAttachment(RootComponent);


}

void APortalActor::InitializePortalData(UMapNode* NodeData)
{
	TargetNodeData = NodeData;
	bIsStageExitPortal = false;

	if (TargetNodeData)
	{
		UpdatePortalWidget();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("다음 맵 노드 정보가 없습니다."));
	}
}

void APortalActor::ActivateAsStageExitPortal()
{
	TargetNodeData = nullptr;
	bIsStageExitPortal = true;

	SetActorEnableCollision(true);

	UpdatePortalWidget();
}

void APortalActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASPGASPlayerCharacter* Player = Cast<ASPGASPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	UMapManagerSubsystem* MapManager = GetGameInstance()->GetSubsystem<UMapManagerSubsystem>();
	if (!MapManager)
	{
		UE_LOG(LogTemp, Log, TEXT("MapManager를 찾을 수 없습니다."));
		return;
	}

	if (bIsStageExitPortal)
	{
		// 1. 스테이지 출구 포탈인 경우 (보스 맵 클리어)
		MapManager->GoToNextStage();
	}
	else if (TargetNodeData)
	{
		// 2. 일반 노드 포탈인 경우
		MapManager->TravelToNode(TargetNodeData);
	}
	SetActorEnableCollision(false);
}



