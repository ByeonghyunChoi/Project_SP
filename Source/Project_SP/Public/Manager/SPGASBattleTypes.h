#pragma once

#include "CoreMinimal.h"
#include "SPGASBattleTypes.generated.h"

//내부 로직 제어용 Enum
UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Ready,              // 전투 진입 및 초기화
	DetermineNextTurn,  // 다음 턴 주인공 결정
	ActiveTurn,         // 현재 유닛 행동 중
	ActionExecuting,    // 행동 실행 중
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
	Single      UMETA(DisplayName = "단일기"), // 단일 (화살표 1개)
	Area        UMETA(DisplayName = "광역기"), // 광역 (모두 화살표)
	All			UMETA(DisplayName = "전체 타겟"),
	Self        UMETA(DisplayName = "나 자신"),          // 자신 (버프 등)
	Random      UMETA(DisplayName = "랜덤")         // 랜덤 (화살표 1개)
};


// 순수 UI 정보
USTRUCT(BlueprintType)
struct FEffectUIData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bIsBuff = false; // true면 버프(파란 테두리 등), false면 디버프(빨간 테두리)
};

// 최종적으로 전달될 UI 정보
USTRUCT(BlueprintType)
struct FUIStatusEffectData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	FText StatusDisplayName;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> StatusIcon;

	UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (MultiLine = true))
	FText StatusDescription;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	bool bIsBuff = false;

	// ASC에서 실시간으로 빼온 남은 턴 수
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	int32 RemainingTurns = 0;
};