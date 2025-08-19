#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ActionData.h"
#include "WeaponSystemComponent.generated.h"

class UWeapon;
class APlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, UWeapon*, NewWeapon);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_SP_API UWeaponSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponSystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	APlayerCharacter* OwnerPlayer;

	// 패링 창이 열려있는지 여부
	bool bIsParryWindowOpen;
	// 현재 공격해오는 적
	TWeakObjectPtr<ACombatPawn> ParryAttacker;
	// 현재 들어오는 공격의 속성
	EDamageType ParryAttackType;



public:
	//무기 교체 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Weapon System|Events")
	FOnWeaponEquipped OnWeaponEquipped;

	// 플레이어가 소유한 3가지 무기 (에디터에서 설정)
	UPROPERTY(EditAnywhere, Instanced, Category = "Weapon System")
	TMap<EDamageType, UWeapon*> Weapons;

	// 현재 장착 중인 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon System")
	UWeapon* CurrentWeapon;

	// 플레이어 턴에 무기를 교체하는 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon System")
	void EquipWeapon(EDamageType WeaponTypeToEquip);

	// 몬스터 턴에 패링(스위치)을 시도하는 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon System")
	bool AttemptParry(EDamageType WeaponTypeToSwitch);

	// 패링 창 이벤트 핸들러
	UFUNCTION()
	void HandleParryWindowChanged(ACombatPawn* Attacker, EDamageType AttackType, bool bIsWindowOpen);

};