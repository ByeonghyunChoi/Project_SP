// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ActionData.h"
#include "WeaponSystemComponent.generated.h"

class UWeapon;
class APlayerCharacter;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

public:	
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

		
};
