//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "SubSystem/SoundManagerSubsystem.h"
//#include "Kismet/GameplayStatics.h"
//
//void USoundManagerSubsystem::Deinitialize()
//{
//	if (FieldAudioComp && FieldAudioComp->IsValidLowLevel()) FieldAudioComp->Stop();
//	if (BattleAudioComp && BattleAudioComp->IsValidLowLevel()) BattleAudioComp->Stop();
//	Super::Deinitialize();
//}
//
//void USoundManagerSubsystem::PlayFieldBGM(USoundBase* NewBGM)
//{
//	if (!NewBGM) return;
//
//	LastFieldBGMAsset = NewBGM;
//
//	// 이미 똑같은 노래가 나오고 있다면 무시 (맵 이동해도 음악 유지)
//	if (FieldAudioComp && FieldAudioComp->IsPlaying() && FieldAudioComp->Sound == NewBGM)
//	{
//		FieldAudioComp->SetVolumeMultiplier(CurrentBGMVolume);
//		return;
//	}
//
//	// 기존 음악 끄기
//	if (FieldAudioComp)
//	{
//		FieldAudioComp->Stop();
//	}
//
//	// 새 음악 장착 및 재생 (Fade In)
//	FieldAudioComp = UGameplayStatics::CreateSound2D(GetGameInstance(), NewBGM);
//	if (FieldAudioComp)
//	{
//		FieldAudioComp->Play();
//		FieldAudioComp->FadeIn(FadeDuration, CurrentBGMVolume);
//	}
//}
//
//void USoundManagerSubsystem::SwitchToBattleBGM(USoundBase* BattleBGM)
//{
//	// 1. 필드 음악 줄이기 (끄지는 않고 볼륨만 0으로 -> 나중에 이어 들으려면)
//	// 완전히 껐다 켜고 싶으면 Stop() 하셔도 됩니다. 여기선 볼륨만 줄입니다.
//	if (FieldAudioComp && FieldAudioComp->IsPlaying())
//	{
//		FieldAudioComp->FadeOut(FadeDuration, 0.0f);
//	}
//
//	// 2. 전투 음악 켜기
//	if (BattleBGM)
//	{
//		// 기존 전투 컴포넌트 재사용 혹은 재생성
//		if (BattleAudioComp) BattleAudioComp->Stop();
//
//		BattleAudioComp = UGameplayStatics::CreateSound2D(GetGameInstance(), BattleBGM);
//		if (BattleAudioComp)
//		{
//			BattleAudioComp->Play();
//			BattleAudioComp->FadeIn(FadeDuration, CurrentBGMVolume);
//		}
//	}
//}
//
//void USoundManagerSubsystem::ReturnToFieldBGM()
//{
//	// 1. 전투 음악 끄기 (Fade Out)
//	if (BattleAudioComp && BattleAudioComp->IsPlaying())
//	{
//		BattleAudioComp->FadeOut(FadeDuration, 0.0f);
//	}
//
//	PlayFieldBGM(LastFieldBGMAsset);
//}
//
//void USoundManagerSubsystem::SetBGMVolume(float NewVolume)
//{
//	CurrentBGMVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
//
//	// 1. 필드 음악 볼륨 조절
//	if (FieldAudioComp && FieldAudioComp->IsValidLowLevel())
//	{
//		FieldAudioComp->SetVolumeMultiplier(CurrentBGMVolume);
//	}
//
//	// 2. 전투 음악 볼륨 조절
//	if (BattleAudioComp && BattleAudioComp->IsValidLowLevel())
//	{
//		BattleAudioComp->SetVolumeMultiplier(CurrentBGMVolume);
//	}
//}
//
