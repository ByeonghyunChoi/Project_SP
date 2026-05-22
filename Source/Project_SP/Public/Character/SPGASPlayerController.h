// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Manager/SPGASBattleTypes.h" 
#include "GameplayEffectTypes.h"
#include "SPGASPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattlePointUpdatedDelegate, int32, CurrentBP, int32, MaxBP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStateChangedDelegate, ESelectedActionType, NewActionState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChangedDelegate, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerExpChangedDelegate, float, CurrentExp, float, MaxExp, float, ExpPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLevelChangedDelegate, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChangedDelegate, FGameplayTag, NewWeaponTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeInterferenceChangedDelegate, bool, bIsActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerTurnStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputProcessedDelegate, FGameplayTag, InputTag, bool, bIsSuccess);
// 입력 액션과 태그를 매핑하는 구조체
USTRUCT(BlueprintType)
struct FSPInputConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UInputAction> InputAction;

	UPROPERTY(EditAnywhere)
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<class USoundBase> ValidSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<class USoundBase> InvalidSound;
};

UCLASS()
class PROJECT_SP_API ASPGASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASPGASPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	// ASC 시스템 초기화 및 태그 이벤트 등록
	void InitAbilitySystem(APawn* InPawn);

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> FieldMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> BattleMappingContext;

	// 필드용 액션 (공격, 상호작용 등)
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FSPInputConfig> FieldInputConfigs;

	// 전투용 액션 (무기 교체 1~3, 행동 선택 Q/W/E)
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FSPInputConfig> BattleInputConfigs;

	// 필드 이동 (WASD)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> MoveAction;

	// [중요] 전투 중 타겟 변경 (A/D or Left/Right)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> BattleNavigateAction;

	// 현재 들고 있는 무기 태그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	FGameplayTag CurrentWeaponTag;

	// 현재 선택된 행동 (공격/스킬/패링)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	ESelectedActionType CurrentSelectedAction;

	// 현재 타겟 선택 모드인가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Targeting")
	bool bIsSelectingTarget = false;

	// 현재 가리키고 있는 타겟의 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Targeting")
	int32 CurrentTargetIndex = 0;

	// 현재 공격 가능한 적 목록 (살아있는 적)
	TArray<TWeakObjectPtr<AActor>> AvailableTargets;

	//현재 선택된 스킬의 타겟팅 타입
	ETargetingType CurrentTargetingType = ETargetingType::Single;

	//전투 중 마우스 클릭 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> BattleClickAction;

	// 패링 액션
	UPROPERTY(EditAnywhere, Category = "Input | RealTime")
	TObjectPtr<class UInputAction> ParryAction;

	//필드 용 UI
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> FieldHUDClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> FieldHUDWidget;

	//전투 용 UI
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> BattleHUDClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class UUserWidget> BattleHUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat | TimeMagic")
	TObjectPtr<class UMaterialParameterCollection> TimeMagicMPC;

protected:
	// 필드 이동 처리
	void OnMove(const FInputActionValue& Value);

	// 전투 타겟 변경 처리 
	void OnBattleNavigate(const FInputActionValue& Value);

	// 필드 액션 
	void OnFieldInputPressed(FGameplayTag InputTag);

	// 전투/필드 상태 변경 감지 
	void OnBattleTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 내 턴인지 확인
	bool IsMyTurn() const;

	// 타겟팅 시작
	void StartTargetSelection();

	// 타겟팅 취소
	void CancelTargetSelection();

	// 현재 타겟의 하이라이트 켜기/끄기
	void HighlightCurrentTarget(bool bHighlight);

	// 최종 확정 및 스킬 실행
	void ConfirmTargetAndExecute();

	// 실제 실행 함수
	void ExecuteBattleAbility(ESelectedActionType ActionType, AActor* TargetActor);

	//마우스 클릭 시 실행될 함수
	void OnBattleClick(const FInputActionValue& Value);

	// 패링 키 클릭 시 실행 함수
	void OnParryPressed(const FInputActionValue& Value);

	//배틀 포인트 변경시 실행될 함수
	void OnBattlePointChanged(const FOnAttributeChangeData& Data);

	//배틀 포인트 최대값 변경시 실행될 함수
	void OnMaxBattlePointChanged(const FOnAttributeChangeData& Data);

	// 플레이어의 행동 상태 변경을 관리할 함수
	void SetCurrentSelectedAction(ESelectedActionType NewAction);

	// 시간 간섭 상태 변경 감지 함수
	void OnTimeInterferenceTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 경험치 변경 감지 함수
	void OnExperienceAttributeChanged(const FOnAttributeChangeData& Data);

	// 레벨 변경 감지 함수
	void OnLevelAttributeChanged(const FOnAttributeChangeData& Data);

	//턴 시작 감지 함수
	void OnTurnActiveTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 키보드 입력을 감지해 소리를 재생하는 함수
	void PlayActionSound(FGameplayTag InputTag, bool bIsSuccess);

	// 전투 중 입력에 따라 UI 상태를 바로 갱신하는 함수
	void HandleInputFeedback(FGameplayTag InputTag, bool bIsSuccess);

public:
	// 무기 교체 처리
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ProcessWeaponSwitch(FGameplayTag NewWeaponTag);
	// 현재 선택한 무기가 뭔지 확인하는 용도의 Getter 함수
	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetCurrentWeaponTag() const { return CurrentWeaponTag; }
	// 전투 액션 (무기교체 or 행동선택 -> 타겟팅 -> 확정)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnBattleInputPressed(FGameplayTag InputTag);
	//쿨타임 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Combat|Cooldown")
	int32 GetSkillCooldownTurns(FGameplayTag SkillTag) const;
	//현재 배틀 포인트를 가져오는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetCurrentBP() const;
	//최대 배틀 포인트를 가져오는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetMaxBP() const;
	//현재 시간의 힘을 가져오는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetCurrentTimePower() const;
	//행동의 Cost를 가져오는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetSkillCost(FGameplayTag ActionTag) const;
	// HP 퍼센트 가져오기 (0.0 ~ 1.0)
	UFUNCTION(BlueprintPure, Category = "Combat | UI")
	float GetHealthPercent() const;
	// 시간의 힘 퍼센트 가져오기 (0.0 ~ 1.0)
	UFUNCTION(BlueprintPure, Category = "Combat | UI")
	float GetTimePowerPercent() const;

	//플레이어 현재 레벨 가져오기
	UFUNCTION(BlueprintPure, Category = "Growth")
	int32 GetCurrentPlayerLevel() const;

	//전투 UI 활성화 함수
	UFUNCTION(BlueprintCallable, Category = "Combat | UI")
	void SetupAndShowBattleUI();

	//전투 UI 비활성화, 필드 UI 활성화 함수
	UFUNCTION(BlueprintCallable, Category = "Combat | UI")
	void HideBattleUIAndShowFieldUI();

	//배틀 포인트 변화를 알리는 함수
	UFUNCTION(BlueprintCallable)
	void RefreshBattlePointUI();

	//턴 순서 UI를 업데이트 하는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat | UI")
	void UpdateTurnTimelineUI(const TArray<AActor*>& NormalTurns, const TArray<AActor*>& VIPTurns, int32 CycleEndIndex);

	// 상점에서 아이템을 구매할 때 UI가 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool BuyShopItem(const FShopItemRow& ItemData);

	// 블루프린트에서 UI 필터를 켜고 끄는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat | UI")
	void ToggleTimeInterferenceUI(bool bIsActive);

	UFUNCTION(BlueprintCallable, Category = "Growth | UI")
	void RefreshExpUI();

	UFUNCTION(BlueprintPure, Category = "Combat | Oparts")
	const class UOpartsDefinition* GetCurrentOpartsDefinition() const;

	//보스 전용 UI를 키는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "UI | Boss")
	void ShowBossUI(class ASPGASMonsterCharacter* BossCharacter);

	//보스 전용 UI를 끄는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "UI | Boss")
	void HideBossUI();

	//컷신 전용 모든 UI를 끄는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat | UI")
	void SetAllHUDVisibility(bool bIsVisible);

	// UI가 이 컨트롤러를 통해 상인을 찾아갈 수 있도록 길을 열어줍니다.
	UPROPERTY(BlueprintReadWrite, Category = "Shop")
	class AMerchantNPC* CurrentMerchant;

private:
	UPROPERTY()
	TObjectPtr<class UAbilitySystemComponent> CachedASC;

public:
	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnBattlePointUpdatedDelegate OnBattlePointUIUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnActionStateChangedDelegate OnActionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnTargetChangedDelegate OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Growth | UI")
	FOnPlayerExpChangedDelegate OnPlayerExpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Growth | UI")
	FOnPlayerLevelChangedDelegate OnPlayerLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnWeaponChangedDelegate OnWeaponChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnTimeInterferenceChangedDelegate OnTimeInterferenceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnPlayerTurnStartedDelegate OnPlayerTurnStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat | UI")
	FOnInputProcessedDelegate OnInputProcessed;

};