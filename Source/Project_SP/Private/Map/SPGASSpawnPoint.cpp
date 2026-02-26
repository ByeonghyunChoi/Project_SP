// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/SPGASSpawnPoint.h"

ASPGASSpawnPoint::ASPGASSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 설정
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 에디터에서 눈에 잘 띄게 화살표 추가
#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->ArrowSize = 1.0f;
	ArrowComponent->SetHiddenInGame(true); // 게임 중에는 숨김
#endif
}

