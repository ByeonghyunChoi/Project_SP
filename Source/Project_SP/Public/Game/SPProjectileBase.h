// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tag/SPGameplayTags.h"
#include "SPProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECT_SP_API ASPProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPProjectileBase();

protected:

	// 데미지를 주는 진짜 투사체 충돌체 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USphereComponent> DamageCollision;

	// 플레이어의 패링을 미리 감지할 대형 충돌체 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USphereComponent> ParryCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

public:	
	// 이 투사체의 속성 (수르트, 펜리르, 요르문간드)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ExposeOnSpawn = "true"))
	FGameplayTag ElementTag;

	// 패링 당했는지 여부 
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsParried = false;

	// 투사체를 쏜 주인 기록
	UPROPERTY(BlueprintReadWrite, Category = "Combat", meta = (ExposeOnSpawn = "true"))
	AActor* ShooterActor;

};
