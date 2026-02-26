#pragma once

#include "CoreMinimal.h"
#include "SPGASBattleTypes.generated.h"

//내부 로직 제어용 Enum
UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Ready,              // 전투 진입 및 초기화
	DetermineNextTurn,  // 다음 턴 주인공 결정
	ActiveTurn,         // 현재 유닛 행동 중 (입력 대기 포함)
	ActionExecuting,    // 행동 실행 중 (몽타주 재생 등)
	Victory,            // 플레이어 승리
	Defeat              // 플레이어 패배
};

// 전투 행동 타입 (공격, 스킬, 패링)
UENUM(BlueprintType)
enum class ESelectedActionType : uint8
{
	None,
	NormalAttack,
	WeaponSkill,
	ParrySkill
};

//타겟팅 타입 정의
UENUM(BlueprintType)
enum class ETargetingType : uint8
{
	Single      UMETA(DisplayName = "Single Target"), // 단일 (화살표 1개)
	Area        UMETA(DisplayName = "Area of Effect"), // 광역 (모두 화살표)
	Self        UMETA(DisplayName = "Self"),          // 자신 (버프 등)
	Random      UMETA(DisplayName = "Random")         // 랜덤 (화살표 1개)
};

