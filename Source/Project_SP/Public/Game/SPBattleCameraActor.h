// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPBattleCameraActor.generated.h"

class USpringArmComponent;
class UCameraComponent;
class APlayerController;

UCLASS()
class PROJECT_SP_API ASPBattleCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPBattleCameraActor();

	// 전투 시작 시 게임모드에서 호출하여 PC와 기본 카메라(돌아올 곳)를 기억하게 합니다.
	UFUNCTION(BlueprintCallable, Category = "Battle | Camera")
	void InitCameraManager(APlayerController* PC);

	// 🌟 타겟에게 카메라를 줌인!
	UFUNCTION(BlueprintCallable, Category = "Battle | Camera")
	void FocusOnTarget(AActor* TargetActor, float BlendTime = 0.0f);

	// 🌟 연출이 끝나고 원래 시점(기본 카메라)으로 복귀!
	UFUNCTION(BlueprintCallable, Category = "Battle | Camera")
	void ReturnToMainView(float BlendTime = 0.0f);

protected:
	// 지지대 (거리, 각도 조절용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraSpringArm;

	// 실제 촬영할 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> MainCamera;

private:
	// 플레이어 컨트롤러 캐싱 (화면 전환 명령을 내리기 위해 필요)
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;

	// 연출이 끝난 후 돌아갈 원래 화면 (플레이어나 전투 메인 카메라)
	UPROPERTY()
	TObjectPtr<AActor> DefaultViewTarget;

};
