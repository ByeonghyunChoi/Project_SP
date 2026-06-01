// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SPCutsceneWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Animation/WidgetAnimation.h"

float USPCutsceneWidget::PlayCutsceneAnimation(int32 CutsceneIndex)
{
	StopAllAnimations();

	if (CutsceneAnimMap.Contains(CutsceneIndex))
	{
		UWidgetAnimation* AnimToPlay = CutsceneAnimMap[CutsceneIndex];
		if (AnimToPlay)
		{
			PlayAnimation(AnimToPlay);
			UE_LOG(LogTemp, Warning, TEXT("[Cutscene] %d번 컷신 재생!"), CutsceneIndex);

			// 🌟 핵심: 애니메이션의 총 길이(초)를 반환합니다!
			return AnimToPlay->GetEndTime();
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[Cutscene] %d번 컷신이 없습니다!"), CutsceneIndex);
	return 0.0f; // 실패 시 0초 반환
}

void USPCutsceneWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CutsceneAnimMap.Empty();

	if (Anim_Cutscene_01) CutsceneAnimMap.Add(1, Anim_Cutscene_01);
	if (Anim_Cutscene_02) CutsceneAnimMap.Add(2, Anim_Cutscene_02);
	if (Anim_Cutscene_03) CutsceneAnimMap.Add(3, Anim_Cutscene_03);
	if (Anim_Cutscene_04) CutsceneAnimMap.Add(4, Anim_Cutscene_04);
	if (Anim_Cutscene_05) CutsceneAnimMap.Add(5, Anim_Cutscene_05);
	if (Anim_Cutscene_06) CutsceneAnimMap.Add(6, Anim_Cutscene_06);
	if (Anim_Cutscene_07) CutsceneAnimMap.Add(7, Anim_Cutscene_07);
	if (Anim_Cutscene_08) CutsceneAnimMap.Add(8, Anim_Cutscene_08);
	if (Anim_Cutscene_09) CutsceneAnimMap.Add(9, Anim_Cutscene_09);
	if (Anim_Cutscene_10) CutsceneAnimMap.Add(10, Anim_Cutscene_10);
	if (Anim_Cutscene_11) CutsceneAnimMap.Add(11, Anim_Cutscene_11);
	if (Anim_Cutscene_12) CutsceneAnimMap.Add(12, Anim_Cutscene_12);
	if (Anim_Cutscene_13) CutsceneAnimMap.Add(13, Anim_Cutscene_13);
	if (Anim_Cutscene_14) CutsceneAnimMap.Add(14, Anim_Cutscene_14);
	if (Anim_Cutscene_15) CutsceneAnimMap.Add(15, Anim_Cutscene_15);
}
