#pragma once

#include "CoreMinimal.h"
#include "Character/CombatPawn.h"
#include "Interface/InteractableInterface.h"
#include "PlayerCharacter.generated.h"

class UWeaponSystemComponent;
class UFieldActionComponent;
class UPlayerCombatControlComponent;
class UInventoryComponent;
class UEquipmentSystemComponent;
class USphereComponent;
class UInputAction;
// 오파츠들
class UCrystalSkullOparts;
class UJadeClockOparts;
class UGoldBugOparts;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableTargetChanged, const FText&, InteractText);

UCLASS()
class PROJECT_SP_API APlayerCharacter : public ACombatPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 필드 모드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
	UFieldActionComponent* FieldActionComp;
	// 무기 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWeaponSystemComponent> WeaponSystemComponent;
	// 전투 행동 제어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Brain")
	TObjectPtr<UPlayerCombatControlComponent> CombatControlComponent;
	// 인벤토리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;
	// 장비 시스템 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UEquipmentSystemComponent> EquipmentSystemComponent;

	// 오파츠 컴포넌트들----------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	TObjectPtr<UCrystalSkullOparts> CrystalSkull;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	TObjectPtr<UJadeClockOparts> JadeClock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Oparts")
	TObjectPtr<UGoldBugOparts> GoldBug;
	// ----------------------------------------

	//Interaction Section
protected:
	//상호작용 볼륨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<USphereComponent> InteractionVolume;

	//상호작용 대상
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction")
	TArray<TScriptInterface<IInteractableInterface>> OverlappedInteractables;

	//상호작용 IA
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	//필드 어택 IA
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FieldAttackAction;

	//상호작용 UI 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableTargetChanged OnInteractableTargetChanged;

	//상호작용 입력이 눌렸을 때 호출될 함수
	void OnInteractInput();

	//필드 공격 입력이 눌렸을 때 호출될 함수
	void OnFieldAttackInput();

	//오버랩 시작 이벤트
	UFUNCTION()
	void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//오버랩 종료 이벤트
	UFUNCTION()
	void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	virtual void OnTurnBegin(const TArray<ACombatPawn*>& PotentialTargets) override;

	// 모드 전환 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Modes")
	void OnEnterFieldMode();

	UFUNCTION(BlueprintImplementableEvent, Category = "Modes")
	void OnEnterBattleMode();

	//UI 갱신 함수
	void UpdateInteractionUI();

	UFUNCTION(BlueprintCallable, Category = "Oparts")
	void LogOpartsActiveState() const; // 오파츠 상태를 로그로 출력하는 함수 선언
};