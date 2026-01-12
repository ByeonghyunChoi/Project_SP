//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Engine/DataTable.h"
//#include "ItemData.generated.h"
//
//UENUM(BlueprintType)
//enum class EItemType : uint8
//{
//	EIT_None UMETA(DisplayName = "None"),
//	EIT_Artifact UMETA(DisplayName = "Artifact"),
//	EIT_Material UMETA(DisplayName = "Material"),
//	// 향후 추가될 아이템이 있으면 이 부분에 추가
//};
//
//USTRUCT(BlueprintType)
//struct FItemData : public FTableRowBase
//{
//	GENERATED_BODY()
//
//public:
//    FItemData()
//        : ItemID(NAME_None)
//        , ItemType(EItemType::EIT_None) // 기본값 유지
//        , ItemName(FText::GetEmpty())
//        , ItemDescription(FText::GetEmpty())
//        , ItemIcon(nullptr)
//        , bCanStack(false)
//    {
//    }
//
//    // --- 2. 파라미터 생성자 (Parameterized Constructor) ---
//    FItemData(
//        FName InItemID,
//        EItemType InItemType,
//        FText InItemName,
//        FText InItemDescription,
//        TSoftObjectPtr<class UTexture2D> InItemIcon,
//        bool InbCanStack
//    )
//        : ItemID(InItemID)
//        , ItemType(InItemType)
//        , ItemName(InItemName)
//        , ItemDescription(InItemDescription)
//        , ItemIcon(InItemIcon)
//        , bCanStack(InbCanStack)
//    {
//    }
//
//	// 아이템 고유 ID
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
//	FName ItemID;
//
//	// 아이템 유형
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
//	EItemType ItemType = EItemType::EIT_None;
//
//	// 아이템 이름, UI에 표시
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
//	FText ItemName;
//
//	// 아이템 설명.
//	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
//	FText ItemDescription;
//
//	// 아이템 아이콘.
//	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
//	TSoftObjectPtr<class UTexture2D> ItemIcon;
//
//	// 아이템이 인벤토리에서 중첩될 수 있는지 여부.
//	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
//	bool bCanStack;
//};
