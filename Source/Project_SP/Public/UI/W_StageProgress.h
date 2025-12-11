#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/CombatTypes.h"
#include "W_StageProgress.generated.h"

class UHorizontalBox;
class UTextBlock;
class UW_StageNode;

UCLASS()
class PROJECT_SP_API UW_StageProgress : public UUserWidget
{
    GENERATED_BODY()

public:
    // 맵 데이터를 받아서 노드들을 생성하는 함수
    UFUNCTION(BlueprintCallable)
    void InitializeMap(int32 ChapterIndex, int32 StageIndex, const TArray<EMapType>& StageList, int32 CurrentStageIndex);

    // 플레이어가 이동했을 때 화살표만 갱신
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerPosition(int32 NewStageIndex);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ChapterText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> NodeContainer;

    // 노드 위젯 클래스 (BP_W_StageNode 할당)
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    TSubclassOf<UW_StageNode> NodeWidgetClass;

    // 생성된 노드들을 저장해두는 배열
    UPROPERTY()
    TArray<TObjectPtr<UW_StageNode>> CreatedNodes;
};