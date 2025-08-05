// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArtifactTypes.h"
#include "ArtifactData.h"
#include "OrpartsData.h"
#include "InventoryComponent.generated.h"

// 전방 선언
class UArtifactSystemComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_SP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
    UInventoryComponent();

    // 획득
    UFUNCTION(BlueprintCallable)
    void AddArtifact(const FArtifactData& NewArtifact);

    UFUNCTION(BlueprintCallable)
    void AddOrparts(const FOrpartsData& NewOrparts);

    // 장착
    UFUNCTION(BlueprintCallable)
    bool EquipArtifact(FName ArtifactID);

    UFUNCTION(BlueprintCallable)
    bool EquipOrparts(FName OrpartsID);

    // 해제
    UFUNCTION(BlueprintCallable)
    void UnequipArtifact(EArtifactType Type);

    UFUNCTION(BlueprintCallable)
    void UnequipOrparts();

    // 조회
    UFUNCTION(BlueprintPure)
    const TArray<FArtifactData>& GetArtifacts() const { return ArtifactInventory; }

    UFUNCTION(BlueprintPure)
    const TArray<FOrpartsData>& GetOrparts() const { return OrpartsInventory; }
protected:
	virtual void BeginPlay() override;

private:	
    UPROPERTY()
	TArray<FArtifactData> ArtifactInventory; // 보유한 아티팩트 목록

    UPROPERTY()
	TArray<FOrpartsData> OrpartsInventory; // 보유한 오파츠 목록

    UPROPERTY()
	UArtifactSystemComponent* ArtifactSystem; // 아티팩트 시스템 컴포넌트

	const FArtifactData* FindArtifactByID(FName ArtifactID) const; // ID로 아티팩트 조회
	const FOrpartsData* FindOrpartsByID(FName OrpartsID) const; // ID로 오파츠 조회
};
