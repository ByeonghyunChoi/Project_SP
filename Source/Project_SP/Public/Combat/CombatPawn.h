#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterStatsComponent.h"
#include "BattleTurnComponent.h"
#include "Data/ActionData.h"
#include "CombatPawn.generated.h"

class UGameEventComponent;
class UStatusEffectComponent;

UENUM(BlueprintType)
enum class ECombatPawnState : uint8
{
	Idle UMETA(DisplayName = "대기"),
	SelectingAction UMETA(DisplayName = "행동 선택 중"),
	SelectingTarget UMETA(DisplayName = "타겟 선택 중"),
	PerformingAction UMETA(DisplayName = "행동 수행 중"),
	Defeated UMETA(DisplayName = "사망"),
};

UENUM(BlueprintType)
enum class EFaction : uint8
{
	Player UMETA(DisplayName = "플레이어"),
	Enemy UMETA(DisplayName = "적"),
	None UMETA(DisplayName = "None")
};

UCLASS()
class PROJECT_SP_API ACombatPawn : public ACharacter
{
	GENERATED_BODY()

public:
	ACombatPawn();

protected:
	virtual void BeginPlay() override;
	// 언리얼 데미지 시스템의 진입점. 여기에서 체력 감소 로직 및 이벤트 브로드캐스트가 이루어집니다.
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// --- 전투원의 현재 내부 상태 (Protected - 내부 함수를 통해서만 변경) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	ECombatPawnState CurrentPawnState;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	EFaction CurrentFaction;

public:
	// --- 공통 컴포넌트들 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
	UGameEventComponent* GameEventComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
	UCharacterStatsComponent* StatsComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
	UBattleTurnComponent* BattleTurnComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
	UStatusEffectComponent* StatusEffectComponent;

	// --- 행동(스킬) 관련 데이터 (DataTable을 통해 정의) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Actions")
	UDataTable* AvailableActionsDataTable; // 이 전투원이 사용할 수 있는 모든 행동 데이터가 있는 DataTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Actions")
	TArray<FName> MyActionIDs; // AvailableActionsDataTable에서 이 전투원이 실제로 사용 가능한 행동 ID 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	EDamageType WeaknessType;
	// --- 현재 진행 중인 행동 관련 변수들 ---
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat|Actions")
	FName SelectedActionID; // 현재 선택된 행동의 ID (타겟 선택 등 중간 단계에서 사용)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat|Actions")
	UGameAction* ActiveActionInstance; // 현재 실행 중인 UGameAction 인스턴스 (행동 로직 관리)

	// --- 공통 행동 함수들 ---
	// 내부적으로 캐릭터 상태를 설정하고, 변경 시 OnCombatPawnStateChanged 이벤트를 브로드캐스트
	UFUNCTION(BlueprintCallable)
	void InternalSetCombatPawnState(ECombatPawnState NewState);
	// ActionID에 해당하는 FActionData를 반환 (DataTable에서 조회)
	UFUNCTION(BlueprintPure, Category = "Combat|Actions")
	FActionData GetActionDataByID(FName ActionID) const;
	// 이 전투원이 사용 가능한 모든 행동 데이터 목록을 반환
	UFUNCTION(BlueprintPure, Category = "Combat|Actions")
	TArray<FActionData> GetAllAvailableActions() const;
	// 행동을 선택하는 시작 함수 (플레이어의 UI 클릭, 몬스터의 AI 결정 등)
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	virtual void SelectAction(FName ActionID);
	// 선택된 행동에 대한 타겟이 확정되었을 때 호출되어 실제 행동을 수행
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	void ExecuteConfirmedAction(FName ActionID, const TArray<ACombatPawn*>& ConfirmedTargets);
	// 행동의 시각적/청각적 부분을 블루프린트에서 구현하기 위한 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Actions", meta = (DisplayName = "ExecuteActionVisuals"))
	void K2_ExecuteActionVisuals(const FActionData& ActionData, ACombatPawn* PrimaryTargetPawn);
	// 캐릭터가 사망했을때 호출될 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void K2_OnDefeated();
	//패링 됬을때 호출할 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat", meta = (DisplayName = "OnParried"))
	void K2_OnParried();
	//패링 결과 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
	virtual void ResolveAction();
	// --- 공통 상태 및 속성 접근자 ---
	UFUNCTION(BlueprintPure, Category = "Combat|State")
	ECombatPawnState GetCombatPawnState() const { return CurrentPawnState; }
	UFUNCTION(BlueprintCallable, Category = "Character")
	EFaction GetFaction() const;
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetFaction(EFaction NewFaction);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	UCharacterStatsComponent* GetStatsComponent() const;
	UFUNCTION(BlueprintCallable, Category = "Battle")
	UBattleTurnComponent* GetBattleTurnComponent() const;
	UFUNCTION(BlueprintCallable, Category = "Combat|Events")
	void BroadcastParryWindowEvent(bool bIsWindowOpen);

	// --- 버프/상태 이상 관련 함수 (초기 스켈레톤, 나중에 구현) ---
   // 버프를 적용할 때 호출
   // UFUNCTION(BlueprintCallable, Category = "Combat|StatusEffects")
   // void ApplyBuff(FName BuffID, ACombatPawn* Instigator);
   // 상태 이상을 적용할 때 호출
   // UFUNCTION(BlueprintCallable, Category = "Combat|StatusEffects")
   // void ApplyStatusEffect(FName StatusEffectID, ACombatPawn* Instigator);
};