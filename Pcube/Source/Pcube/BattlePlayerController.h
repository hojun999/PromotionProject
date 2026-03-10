// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BattlePlayerController.generated.h"

enum class EBattleResult : uint8;
class USkillDataAsset;
enum class EBattleState : uint8;
class UBattleControlSubsystem;
class ABattleBaseUnit;
class ABattleAllyUnit;
class ABattleHUD;

UCLASS()
class PCUBE_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	void NotifyBattleHUDReady(ABattleHUD* HUD);
	
private:
	// --- 델리게이트 바인딩 ---
	void BindBattleDelegates();
	void UnbindBattleDelegates();
	
	void BindWidgetRequests();
	
	// --- 이벤트 핸들러 ---
	UFUNCTION()
	void HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit);
	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);
	UFUNCTION()
	void HandleBattleStateChanged(EBattleState NewState);
	UFUNCTION()
	void HandleActionRequested(USkillDataAsset* SkillData);
	UFUNCTION()
	void HandleSkillRequested(USkillDataAsset* Skill);
	UFUNCTION()
	void HandleSkillMenuRequested();
	UFUNCTION()
	void HandleItemMenuRequested();
	UFUNCTION()
	void HandleBattleFinished(EBattleResult Result);
	
	// --- 카메라 / Input 헬퍼 ---
	void FocusViewTarget(AActor* Target, float BlendTime);
	void FocusDefaultBattleCamera(float BlendTime);
	void ApplyInputMode_GameOnly(bool bShowCursor);
	void ApplyInputMode_GameAndUI(UUserWidget* FocusWidget, bool bShowCursor);
	
	// --- 타겟 전환을 위한 Input 바인딩 ---
	void Input_NextTarget();
	void Input_PrevTarget();
	void Input_ConfirmTarget();
	void Input_ClickConfirmTarget();
	void Input_CancelTarget();
	void Input_BackOrCancel();
	
	// --- 포인터 캐시 ---
	UPROPERTY()
	TObjectPtr<UBattleControlSubsystem> BattleSub = nullptr;
	
	UPROPERTY()
	TObjectPtr<ABattleHUD> BattleHUD = nullptr;
	
	void CacheBattleHUD();
};
