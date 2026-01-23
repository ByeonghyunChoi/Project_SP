#pragma once

#include "CoreMinimal.h"
#include "SPGASBattleTypes.generated.h"

//내부 로직 제어용 Enum
UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Ready,              // 전투 진입 및 초기화
	DetermineNextTurn,  // 다음 턴 주인공 결정 (시간 흐름 계산)
	ActiveTurn,         // 현재 유닛 행동 중
	WaitInterruption,   // 궁극기 난입 등 이벤트 대기
	GameOver            // 전투 종료 및 결과 정산
};

// 타임라인 관리를 위한 유닛 데이터
USTRUCT(BlueprintType)
struct FBattleUnit
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> UnitActor;

	float BaseAV = 0.0f;     // 100% 게이지에 필요한 시간 (10000 / Speed)
	float CurrentAV = 0.0f;  // 남은 시간
	int32 Priority = 0;      // 난입 우선순위
	bool bIsResurgence = false; // 재행동 예약 플래그
	bool bIsPlayer = false; // 플레이어 여부
	float Speed = 0.0f; // 속도 비교를 위한 변수

	// 정렬 기준: 우선순위 -> 행동 게이지 -> 속도 -> 진영 -> 고유 값
	bool operator<(const FBattleUnit& Other) const
	{
		// 1순위: 우선순위
		if (Priority != Other.Priority) return Priority > Other.Priority;

		// 2순위: 남은 행동 게이지
		if (!FMath::IsNearlyEqual(CurrentAV, Other.CurrentAV, 0.001f))
		{
			return CurrentAV < Other.CurrentAV;
		}

		// 3순위: 속도 우선
		if (!FMath::IsNearlyEqual(Speed, Other.Speed, 0.001f))
		{
			return Speed > Other.Speed;
		}

		//4순위: 플레이어 우선
		if (bIsPlayer != Other.bIsPlayer) return bIsPlayer > Other.bIsPlayer;

		//5순위: 고유 값
		return UnitActor < Other.UnitActor;
	}
};