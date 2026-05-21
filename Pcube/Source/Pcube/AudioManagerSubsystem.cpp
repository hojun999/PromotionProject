// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManagerSubsystem.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UAudioManagerSubsystem::UAudioManagerSubsystem()
{
	// Sound Class 할당
	static ConstructorHelpers::FObjectFinder<USoundClass> BGMClassAsset(TEXT("/Script/Engine.SoundClass'/Game/Sound/SC_BGM.SC_BGM'"));
	if (BGMClassAsset.Succeeded())
	{
		BGMSoundClass = BGMClassAsset.Object;
	}

	// Sound Class 할당
	static ConstructorHelpers::FObjectFinder<USoundClass> SFXClassAsset(TEXT("/Script/Engine.SoundClass'/Game/Sound/SC_BGM.SC_BGM'"));
	if (SFXClassAsset.Succeeded())
	{
		SFXSoundClass = SFXClassAsset.Object;
	}
	
	// Sound Mix 할당
	static ConstructorHelpers::FObjectFinder<USoundMix> MainMixAsset(TEXT("/Script/Engine.SoundMix'/Game/Sound/SCM_Main.Mix_Main'"));
	if (MainMixAsset.Succeeded())
	{
		MainSoundMix = MainMixAsset.Object;
	}
}

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BGMVolume = 0.5f;
	SFXVolume = 0.5f;
}

// ── 볼륨 적용 내부 함수 ───────────────────────────────────────

void UAudioManagerSubsystem::ApplyBGMVolume()
{
	UWorld* World = GetWorld();
	if (!World || !MainSoundMix || !BGMSoundClass) return;

	// 핵심: Sound Mix를 통해 해당 클래스의 볼륨을 오버라이드
	UGameplayStatics::SetSoundMixClassOverride(
	   World,
	   MainSoundMix,
	   BGMSoundClass,
	   BGMVolume, // 0.0 ~ 1.0f
	   1.0f,      // Pitch
	   0.0f,      // FadeInTime (즉시 반영하려면 0)
	   true       // 기존 설정을 덮어쓸지 여부
	);
	
	// float FinalVolume = (BGMVolume <= 0.0f) ? 0.0001f : BGMVolume;
	//
	// if (BGMSoundClass)
	// {
	// 	BGMSoundClass->Properties.Volume = FinalVolume;
	// }
	//
	// if (BGMComponent)
	// {
	// 	BGMComponent->SetVolumeMultiplier(FinalVolume);
	// }
	
	// if (BGMComponent)
	// {
	// 	if (BGMVolume <= 0.f)
	// 	{
	// 		BGMComponent->SetVolumeMultiplier(0.f);
	// 		bIsBGMMuted = true; // 무음 상태 기록 // 추가됨
	// 	}
	// 	else
	// 	{
	// 		if (bIsBGMMuted)
	// 		{
	// 			// 무음에서 복구 시 - 현재 재생 위치 유지하며 볼륨만 복구
	// 			bIsBGMMuted = false;
	// 			BGMComponent->SetVolumeMultiplier(BGMVolume);
	// 		}
	// 		else
	// 		{
	// 			BGMComponent->SetVolumeMultiplier(BGMVolume);
	// 		}
	// 	}
	// }
}

void UAudioManagerSubsystem::ApplySFXVolume()
{
	if (SFXSoundClass)
	{
		SFXSoundClass->Properties.Volume = SFXVolume;
	}
}

// ── 볼륨 설정 ────────────────────────────────────────────────

void UAudioManagerSubsystem::SetBGMVolume(float Volume)
{
	BGMVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyBGMVolume();
}

void UAudioManagerSubsystem::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySFXVolume();
}

// ── BGM 재생/정지 ─────────────────────────────────────────────

void UAudioManagerSubsystem::PlayBGM(USoundBase* BGMSound, float FadeInSeconds)
{
	if (!BGMSound) return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) return;

	// 이미 같은 사운드면 볼륨만 재적용 (무음 상태 포함) // 수정됨
	if (IsValid(BGMComponent) && BGMComponent->Sound == BGMSound)
	{
		ApplyBGMVolume();
		return;
	}

	// 기존 BGM 정지
	if (BGMComponent)
	{
		BGMComponent->FadeOut(FadeInSeconds * 0.4f, 0.0f);
		BGMComponent = nullptr;
	}

	// 새 BGM 스폰
	BGMComponent = UGameplayStatics::SpawnSound2D(
		World,
		BGMSound,
		1.0f,    // VolumeMultiplier - ApplyBGMVolume에서 재설정
		1.0f,
		0.0f,
		nullptr,
		true,    // bPersistAcrossLevelTransition
		false    // bAutoDestroy
	);

	if (BGMComponent)
	{
		BGMComponent->FadeIn(FadeInSeconds > 0.f ? FadeInSeconds : 0.01f);
		// 현재 볼륨 즉시 반영 (볼륨 0이면 무음으로 시작)
		ApplyBGMVolume();
	}
}

void UAudioManagerSubsystem::StopBGM(float FadeOutSeconds)
{
	if (BGMComponent)
	{
		BGMComponent->FadeOut(FadeOutSeconds, 0.0f);
		BGMComponent = nullptr;
	}
}

// ── SFX 재생 ─────────────────────────────────────────────────

void UAudioManagerSubsystem::PlaySFX(USoundBase* SFXSound)
{
	if (!SFXSound) return;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) return;

	UGameplayStatics::PlaySound2D(
		World,
		SFXSound,
		1.0f,
		1.0f,
		0.0f,
		nullptr,
		nullptr,
		false
	);
}

// 하위 호환용 - 내부적으로 ApplyBGMVolume/ApplySFXVolume으로 위임
void UAudioManagerSubsystem::ApplySoundMixOverride(USoundClass* SoundClass, float Volume)
{
	if (SoundClass)
	{
		SoundClass->Properties.Volume = Volume;
	}
}
