// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SPProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ASPProjectileBase::ASPProjectileBase()
{
	// 데미지 충돌체 세팅
	DamageCollision = CreateDefaultSubobject<USphereComponent>(TEXT("DamageCollision"));
	DamageCollision->InitSphereRadius(15.0f);
	RootComponent = DamageCollision;

	// 패링 감지용 대형 충돌체 세팅
	ParryCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ParryCollision"));
	ParryCollision->InitSphereRadius(100.0f); // 🌟 데미지 판정보다 훨씬 크게!
	ParryCollision->SetupAttachment(RootComponent);

	// 투사체 이동 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1500.0f;
	ProjectileMovement->MaxSpeed = 1500.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
}