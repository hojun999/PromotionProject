// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManagerSubsystem.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BGMVolume = 0.5f;
	SFXVolume = 0.5f;
}

// ── 볼륨 적용 내부 함수 ───────────────────────────────────────

void UAudioManagerSubsystem::ApplyBGMVolume()
{
	// SoundClass 볼륨 설정
	if (BGMSoundClass)
	{
		BGMSoundClass->Properties.Volume = BGMVolume;
	}

	// AudioComponent에도 직접 반영
	if (BGMComponent)
	{
		if (BGMVolume <= 0.f)
		{
			// 볼륨 0 = 완전 무음: VolumeMultiplier 0으로 설정 (Pause 대신)
			// Pause는 IsPlaying()을 false로 만들어 PlayBGM 재진입 시 중복 스폰 유발
			BGMComponent->SetVolumeMultiplier(0.f);
		}
		else
		{
			BGMComponent->SetVolumeMultiplier(BGMVolume);
		}
	}
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

	// 이미 같은 사운드가 재생 중이면 볼륨만 재적용하고 스킵
	if (BGMComponent && BGMComponent->Sound == BGMSound)
	{
		// 볼륨 0으로 무음 상태였을 수 있으니 현재 볼륨 재적용
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
