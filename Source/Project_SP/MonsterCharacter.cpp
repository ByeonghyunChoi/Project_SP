// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterCharacter.h"

// Sets default values
AMonsterCharacter::AMonsterCharacter()
{

}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (MonsterBaseClass)
	{
		MonsterData = NewObject<UMonsterBase>(this, MonsterBaseClass);

		if (MonsterData)
		{
			UE_LOG(LogTemp, Log, TEXT("몬스터 데이터가 생성완료."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("몬스터 데이터가 없습니다."));
	}
}

void AMonsterCharacter::PerformMonsterTurnAction()
{
	if (MonsterData)
	{
		MonsterData->DecideAction();
	}
}


