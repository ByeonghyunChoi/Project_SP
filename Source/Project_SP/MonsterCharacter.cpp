// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterCharacter.h"
#include "NormalMonsterBase.h"
#include "EliteMonsterBase.h"
#include "BossMonsterBase.h"


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
    MonsterAIInstance = CreateDefaultSubobject<UMonsterBase>(TEXT("MonsterAIInstance"));
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    CombatData->SetFaction(EFaction::Enemy);
    
    // MonsterAIClass가 설정되어 있고, MonsterAIInstance가 해당 타입이 아니면 새로 생성
    if (MonsterClass && MonsterAIInstance && MonsterAIInstance->GetClass() != MonsterClass)
    {
        // 기존 MonsterAIInstance는 가비지 컬렉션 대상이 됨
        MonsterAIInstance = NewObject<UMonsterBase>(this, MonsterClass);
        UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: BeginPlay에서 MonsterAIInstance를 템플릿으로 재설정."));
    }
    else if (MonsterClass && !MonsterAIInstance) // MonsterAIInstance가 아직 없다면
    {
        MonsterAIInstance = NewObject<UMonsterBase>(this, MonsterClass);
        UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: BeginPlay에서 MonsterAIInstance가 없어 템플릿으로 생성."));
    }
    else if (MonsterAIInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: 기존 MonsterAIInstance 사용 중 (%s)"), *GetNameSafe(MonsterAIInstance));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AMonsterCharacter: MonsterAIClass와 MonsterAIInstance 모두 설정되지 않았습니다."));
    }
}

#if WITH_EDITOR
void AMonsterCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property != nullptr)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();

        // MonsterAIClass 변수가 변경되었을 때
        if (PropertyName == GET_MEMBER_NAME_CHECKED(AMonsterCharacter, MonsterClass))
        {
            if (MonsterClass)
            {
                // MonsterAIInstance를 선택된 MonsterAIClass 타입으로 새로 생성하여 할당
                MonsterAIInstance = NewObject<UMonsterBase>(this, MonsterClass);
                UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: 에디터에서 MonsterAIClass 변경 감지, MonsterAIInstance 재설정."));
            }
            else
            {
                // MonsterAIClass가 None으로 설정되면 MonsterAIInstance도 None으로
                MonsterAIInstance = nullptr; // 또는 기본 UMonsterBase 인스턴스로
                UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: 에디터에서 MonsterAIClass가 None으로 설정됨."));
            }
        }
    }
}
#endif

void AMonsterCharacter::PerformMonsterTurnAction()
{
	if (MonsterAIInstance)
	{
        MonsterAIInstance->DecideAction();
	}
}



