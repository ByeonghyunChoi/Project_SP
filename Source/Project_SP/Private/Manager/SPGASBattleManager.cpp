// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SPGASBattleManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet/SPGASAttributeSet.h"
#include "Tag/SPGameplayTags.h"

ASPGASBattleManager::ASPGASBattleManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASPGASBattleManager::StartBattle(const TArray<AActor*>& Players, const TArray<AActor*>& Monsters)
{
	Timeline.Empty();

	TArray<AActor*> AllParticipants = Players;
	AllParticipants.Append(Monsters);

	for (AActor* Actor : AllParticipants)
	{
		FBattleUnit NewUnit;
		NewUnit.UnitActor = Actor;

		float Speed = GetSpeedFromActor(Actor);
		NewUnit.BaseAV = 10000.f / Speed;
		NewUnit.CurrentAV = NewUnit.BaseAV;

		Timeline.Add(NewUnit);
	}

	// 초기 상태 설정
	RefreshTimeline();
	SetBattleState(EBattleState::DetermineNextTurn);
}

void ASPGASBattleManager::RequestInterruption(AActor* Interrupter)
{
	for (auto& Unit : Timeline)
	{
		if (Unit.UnitActor == Interrupter)
		{
			// 우선순위를 높여 정렬 시 최상단으로 오게 함
			Unit.Priority = 10;
			// 이미 턴이 진행 중인 유닛보다 우선하기 위해 아주 작은 음수값 부여
			Unit.CurrentAV = -0.01f;
			break;
		}
	}

	// 즉시 타임라인을 갱신하여 UI나 로직에 반영
	RefreshTimeline();

	// 만약 현재가 '시간을 흐르게 하는 중'이었다면 즉시 다음 턴 결정으로 유도 가능
	if (InternalState == EBattleState::DetermineNextTurn)
	{
		SetBattleState(EBattleState::DetermineNextTurn);
	}
}

void ASPGASBattleManager::NotifyTurnEnd()
{
	for (auto& Unit : Timeline)
	{
		if (Unit.UnitActor == CurrentActiveUnit)
		{
			// 턴이 끝났으므로 난입 우선순위 초기화
			Unit.Priority = 0;

			if (Unit.bIsResurgence)
			{
				// 재행동 플래그가 있다면 AV를 0으로 설정해 즉시 다시 행동하게 함
				Unit.CurrentAV = 0.0f;
				Unit.bIsResurgence = false;
				UE_LOG(LogTemp, Warning, TEXT("재행동 발생: %s"), *Unit.UnitActor->GetName());
			}
			else
			{
				// 일반 턴 종료: 다시 100% 거리를 채움
				Unit.CurrentAV = Unit.BaseAV;
			}
			break;
		}
	}

	// 타임라인 다시 정렬 후 다음 턴으로
	RefreshTimeline();
	SetBattleState(EBattleState::DetermineNextTurn);
}

void ASPGASBattleManager::SetBattleState(EBattleState NewState)
{
	if (InternalState == NewState) return;

	InternalState = NewState;
	UpdateInterfaceTags(NewState);

	switch (InternalState)
	{
	case EBattleState::DetermineNextTurn:
		AdvanceTimeToNextTurn();
		break;
	case EBattleState::ActiveTurn:
		//유닛에게 행동 실행 명령
		break;
	}

}

void ASPGASBattleManager::UpdateInterfaceTags(EBattleState NewState)
{
	const FSPGameplayTags& SPTags = FSPGameplayTags::Get();

	if (CurrentActiveUnit)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentActiveUnit))
		{
			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();

			if (NewState == EBattleState::ActiveTurn)
			{
				ASC->AddLooseGameplayTag(SPTags.State_Turn_Active);
			}
			else
			{
				ASC->RemoveLooseGameplayTag(SPTags.State_Turn_Active);
			}
		}
	}

	if (NewState == EBattleState::GameOver)
	{
		for (auto& Unit : Timeline)
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Unit.UnitActor))
			{
				ASI->GetAbilitySystemComponent()->RemoveLooseGameplayTag(SPTags.State_Mode_Battle);
			}
		}
	}
}

void ASPGASBattleManager::RefreshTimeline()
{
	for (auto& Unit : Timeline)
	{
		// 속도를 가져와서 변수에 저장
		float NewSpeed = GetSpeedFromActor(Unit.UnitActor);
		float NewBaseAV = 10000.f / NewSpeed;

		//속도 변화에 따른 남은 시간 보정
		if (Unit.BaseAV > 0)
		{
			Unit.CurrentAV = Unit.CurrentAV * (NewBaseAV / Unit.BaseAV);
		}

		//데이터 업데이트 
		Unit.BaseAV = NewBaseAV;
		Unit.Speed = NewSpeed; 
	}
	// FBattleUnit 구조체에 정의한 operator< 에 의해 정렬됨
	// Priority가 먼저, 그 다음이 CurrentAV 순서
	Timeline.Sort();
}

void ASPGASBattleManager::AdvanceTimeToNextTurn()
{
	if (Timeline.Num() == 0) return;

	// 이미 정렬되어 있으므로 0번 유닛이 가장 빠름
	// Priority가 높은 유닛이 있다면 CurrentAV가 0보다 작을 수도 있음
	float TimeToAdvance = FMath::Max(0.0f, Timeline[0].CurrentAV);

	// 전체 유닛의 AV에서 TimeToAdvance를 차감 (시간이 흐름)
	for (auto& Unit : Timeline)
	{
		Unit.CurrentAV -= TimeToAdvance;
	}

	// 현재 턴 유닛을 확정하고 상태 전환
	CurrentActiveUnit = Timeline[0].UnitActor;
	SetBattleState(EBattleState::ActiveTurn);
}

float ASPGASBattleManager::GetSpeedFromActor(AActor* Actor) const
{
	if (!Actor) return 100.f; // 안전장치: 액터가 없으면 기본값 반환

	// 인터페이스 캐스팅 (우리가 AActor*로 관리하는 이유입니다)
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor);
	if (!ASI) return 100.f;

	// ASC 가져오기
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return 100.f;

	// AttributeSet에서 Speed 값 추출
	// GetNumericAttribute는 버프/디버프가 모두 계산된 '최종 값'을 가져옵니다.
	bool bFound = false;
	float Speed = ASC->GetNumericAttribute(USPGASAttributeSet::GetSpeedAttribute());

	// 속도가 0이면 무한 루프나 나눗셈 오류가 발생하므로 최소값을 보장합니다.
	return FMath::Max(1.0f, Speed);
}




