// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioManagerSubsystem.generated.h"

class USoundClass;
class USoundMix;
class UAudioComponent;
class USoundBase;

/**
 * 게임 전체 오디오 볼륨 관리 및 BGM 재생 담당 (GameInstanceSubsystem)
 * - SoundClass 기반으로 BGM/SFX 볼륨을 독립적으로 조절
 * - BGM 페이드 인/아웃 지원
 */
UCLASS()
class PCUBE_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ── 볼륨 설정 (0.0 ~ 1.0) ─────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="Audio")
	void SetBGMVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="Audio")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
	float GetBGMVolume() const { return BGMVolume; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
	float GetSFXVolume() const { return SFXVolume; }

	// ── BGM 재생/정지 ──────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="Audio")
	void PlayBGM(USoundBase* BGMSound, float FadeInSeconds = 1.5f);

	UFUNCTION(BlueprintCallable, Category="Audio")
	void StopBGM(float FadeOutSeconds = 1.0f);

	// ── SFX 재생 (2D) ──────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="Audio")
	void PlaySFX(USoundBase* SFXSound);

	// ── SoundClass / SoundMix 레퍼런스 (에디터에서 할당) ──────
	// Project Settings > Plugins 또는 Content/Audio 에서
	// SC_BGM, SC_SFX SoundClass 에셋과 SM_Master SoundMix 에셋을 만들어 할당
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio|Setup")
	TObjectPtr<USoundClass> BGMSoundClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio|Setup")
	TObjectPtr<USoundClass> SFXSoundClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio|Setup")
	TObjectPtr<USoundMix> MasterSoundMix = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent = nullptr;

	float BGMVolume = 1.0f;
	float SFXVolume = 1.0f;

	void ApplySoundMixOverride(USoundClass* SoundClass, float Volume);
	void ApplyBGMVolume(); // BGM 볼륨 실제 적용
    void ApplySFXVolume(); // SFX 볼륨 실제 적용
};
