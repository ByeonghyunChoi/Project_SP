// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterCharacter.h"
#include "NormalMonsterBase.h"
#include "EliteMonsterBase.h"
#include "BossMonsterBase.h"


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
    CombatData->SetFaction(EFaction::Enemy);
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (MonsterClass && CombatData && CombatData->GetClass() != MonsterClass)
    {
        CombatData = NewObject<UMonsterBase>(this, MonsterClass);
        UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: BeginPlay에서 CombatDataInstance를 템플릿으로 재설정."));
    }
    else if (!CombatData && MonsterClass)
    {
        // CombatDataInstance가 nullptr인데 템플릿이 설정되어 있으면 새로 생성
        CombatData = NewObject<UMonsterBase>(this, MonsterClass);
        UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: BeginPlay에서 CombatDataInstance가 없어 템플릿으로 생성."));
    }
    else if (!CombatData && !MonsterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AMonsterCharacter: CombatDataInstance와 MonsterBaseClassTemplate 모두 설정되지 않았습니다."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: CombatDataInstance 사용 중 (%s)"), *GetNameSafe(CombatData));
    }
    
}

#if WITH_EDITOR
void AMonsterCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property != nullptr)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();

        // MonsterBaseClassTemplate 변수가 변경되었을 때
        if (PropertyName == GET_MEMBER_NAME_CHECKED(AMonsterCharacter, MonsterClass))
        {
            if (MonsterClass)
            {
                // CombatDataInstance를 선택된 MonsterBaseClassTemplate 타입으로 새로 생성
                // 이렇게 하면 디테일 패널에서 MonsterBaseClassTemplate을 변경하는 즉시
                // CombatDataInstance의 타입과 기본값이 반영됩니다.
                CombatData = NewObject<UCharacterBase>(this, MonsterClass);
                UE_LOG(LogTemp, Log, TEXT("AMonsterCharacter: 에디터에서 MonsterBaseClassTemplate 변경 감지, CombatDataInstance 재설정."));
            }
            else
            {
                // MonsterBaseClassTemplate이 None으로 설정되면 CombatDataInstance도 None으로
                CombatData = nullptr;
            }
        }
    }
}
#endif

void AMonsterCharacter::PerformMonsterTurnAction()
{
	if (CombatData)
	{
		CombatData->DecideAction();
	}
}



