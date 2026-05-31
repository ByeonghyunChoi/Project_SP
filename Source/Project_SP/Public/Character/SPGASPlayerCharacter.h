#pragma once

#include "CoreMinimal.h"
#include "Character/SPGASCharacterBase.h"
#include "GameplayTagContainer.h"
#include "Data/Asset/WeaponAbilityData.h"
#include "Manager/SPGASBattleTypes.h"
#include "Data/SPDialogData.h"
#include "SPGASPlayerCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCameraProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetArmLength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RelativeRotation = FRotator(-45.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableLag = true;

	//카메라 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CameraRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CameraRelativeRotation = FRotator::ZeroRotator;
};


UCLASS()
class PROJECT_SP_API ASPGASPlayerCharacter : public ASPGASCharacterBase
{
	GENERATED_BODY()

public:
	ASPGASPlayerCharacter();

	virtual void PossessedBy(AController* NewController) override;

	void ActivateCombatAbility(FGameplayTag WeaponTag, ESelectedActionType ActionType, AActor* TargetActor);
	
	// 전투 시작 시 호출할 전투 준비 함수
	virtual void OnBattleStarted() override;
	
	//카메라 모드(전투/ 필드) 변경 함수
	void SwitchCameraMode(bool bIsBattle);

	// 카메라 확정 함수
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	//행동을 태그로 감지할 콜백 함수
	virtual void OnActionTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

public:
	// 스탯 상승용 커브 테이블 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data | Growth")
	class UCurveTable* PlayerStatCurve;

	// 레벨업 보상용 데이터 테이블(일단 지금은 비워 둠)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data | Growth")
	class UDataTable* PlayerRewardTable;

	// 광대 선택지 실행 함수
	UFUNCTION(BlueprintCallable, Category = "Event") 
	void ExecuteClownChoice(EClownChoiceType Choice, const TArray<URelicDefinition*>& AllRelicPool);

protected:

	UPROPERTY(EditAnywhere, Category = "GAS | Field")
	TMap<FGameplayTag, TSubclassOf<class UGameplayAbility>> FieldInputAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Field")
	TArray<TSubclassOf<class UGameplayAbility>> FieldPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Battle")
	TMap<FGameplayTag, TSubclassOf<class UGameplayAbility>> BattleInputAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS | Battle")
	TArray<TSubclassOf<class UGameplayAbility>> BattlePassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS | Battle")
	TMap<FGameplayTag, TObjectPtr<UWeaponAbilityData>> WeaponConfigs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	// 실제 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

	//전투용 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCineCameraComponent> CombatCineCamera;

	//무기 교체 시 재생할 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "GAS | Battle | Animation")
	TObjectPtr<UAnimMontage> WeaponSwapMontage;

	FGameplayTag PendingWeaponTag;

protected:
	virtual void BeginPlay() override;

	virtual void OnRep_PlayerState() override;

	void GiveAbilities();

	void GiveWeaponAbilities();

	void OnHealthChanged(const struct FOnAttributeChangeData& Data);

	void OnTimePowerChanged(const struct FOnAttributeChangeData& Data);

	void ApplyLevelStats(int32 TargetLevel, bool bIsLevelUp = false);

	// 레벨 업 시 연출 담당 함수(블루프린트에서 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Player | Growth")
	void OnLevelUpEffect();

	void OnWeaponSwapMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 시간의 힘을 전부 다 썼을 때 연출 함수
	void ExecuteTimeOverSequence();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class USPInteractionComponent> InteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Camera | Profile")
	FCameraProfile FieldCameraSetting;

	UPROPERTY(EditDefaultsOnly, Category = "Camera | Profile")
	FCameraProfile CombatCameraSetting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> WeaponWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> ActionWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> BattlePointWidgetComponent;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat | Weapon")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat | Target")
	TObjectPtr<AActor> LastParriedTarget;

public:
	// Setter
	UFUNCTION(BlueprintCallable)
	void SetCameraProfile(const FCameraProfile& Profile);
	// Getter
	const FCameraProfile& GetFieldCameraProfile() const { return FieldCameraSetting; }
	const FCameraProfile& GetCombatCameraProfile() const { return CombatCameraSetting; }
	TObjectPtr<class UWidgetComponent> GetWeaponWidgetComponent() { return WeaponWidgetComponent; }
	TObjectPtr<class UWidgetComponent> GetActionWidgetComponent() { return ActionWidgetComponent; }
	TObjectPtr<class UWidgetComponent> GetBattlePointWidgetComponent() { return BattlePointWidgetComponent; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	ETargetingType GetTargetingType(FGameplayTag WeaponTag, ESelectedActionType ActionType) const;
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	UWeaponAbilityData* GetWeaponData(FGameplayTag WeaponTag) const;

	// 컨트롤러가 호출할 함수
	void PlayWeaponSwapSequence(FGameplayTag NewTag);

	// 애니메이션 노티파이가 호출할 함수들 (BlueprintCallable 필수)
	UFUNCTION(BlueprintCallable, Category = "Combat | Animation")
	void HandleWeaponHide();

	UFUNCTION(BlueprintCallable, Category = "Combat | Animation")
	void HandleWeaponShow();

	void CheckLevelUp();

	//행동 선택 시 UI 보여주는 연출 이벤트
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Camera | Action")
	void ToggleActionCameraMode(bool bIsSelecting, bool bInstantReset, int32 TargetIndex, ETargetingType TargetType);
	
public:
	// 경험치 획득 함수
	UFUNCTION(BlueprintCallable, Category = "Player | Growth")
	void AddExperience(float ExpAmount);
};