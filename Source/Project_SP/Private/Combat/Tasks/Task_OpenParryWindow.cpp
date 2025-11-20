// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Tasks/Task_OpenParryWindow.h"
#include "Character/CombatPawn.h"
#include "Component/GameEventComponent.h"
#include "Character/MonsterCharacter.h"
#include "Combat/GameAction.h"
#include "TimerManager.h"

void UTask_OpenParryWindow::ExecuteTask_Implementation()
{
    UGameAction* OwningAction = Cast<UGameAction>(GetOuter());

    // Instigator가 있고, 적 진영(몬스터)일 때만 실행
    if (Instigator && OwningAction && Instigator->GetFaction() == EFaction::Enemy)
    {
        const FActionData& ActionData = OwningAction->GetData();

        // 패링 가능 시간이 설정된 스킬인 경우
        if (ActionData.ParryWindowDuration > 0.f)
        {
            if (UGameEventComponent* EventComp = Instigator->GetGameEventComponent())
            {
                // [수정 전] 액션 데이터의 데미지 타입을 패링 조건으로 사용
                // EDamageType ParryType = ActionData.DamageType;

                // [수정 후] 몬스터 본체의 약점 타입을 패링 조건으로 사용
                EDamageType RequiredParryType = EDamageType::Fenrir; // 기본값

                // Instigator를 몬스터로 캐스팅하여 약점 정보 가져오기
                if (AMonsterCharacter* Monster = Cast<AMonsterCharacter>(Instigator))
                {
                    RequiredParryType = Monster->GetWeaknessType();
                    UE_LOG(LogTemp, Log, TEXT("Parry Window Open: Using Monster Weakness [%s]"), *UEnum::GetValueAsString(RequiredParryType));
                }
                else
                {
                    // 몬스터가 아닌 경우(예외 상황), 기존대로 액션 데이터 사용하거나 기본값 사용
                    RequiredParryType = ActionData.DamageType;
                }

                // 이벤트 방송: 이제 '공격 속성'이 아닌 '몬스터 약점'을 인자로 보냅니다.
                EventComp->BroadcastParryWindowOpened(Instigator, RequiredParryType, ActionData.ParryWindowDuration);

                // 타이머 설정 (기존 코드 유지)
                if (UWorld* World = GetWorld())
                {
                    World->GetTimerManager().SetTimer(
                        ParryWindowTimerHandle,
                        this,
                        &UTask_OpenParryWindow::OnParryWindowTimerEnd,
                        ActionData.ParryWindowDuration,
                        false
                    );
                }
                return;
            }
        }
    }

    // 조건이 맞지 않으면 즉시 종료
    FinishTask();
}

void UTask_OpenParryWindow::OnParryWindowTimerEnd()
{
	if (Instigator)
	{
		if (UGameEventComponent* EventComp = Instigator->GetGameEventComponent())
		{
			// "패링 창 닫힘"을 방송합니다.
			EventComp->BroadcastParryWindowClosed(Instigator);
		}
	}

	// 이제 패링 창의 열림과 닫힘에 대한 모든 임무가 끝났으므로, 작업을 종료합니다.
	FinishTask();
}
