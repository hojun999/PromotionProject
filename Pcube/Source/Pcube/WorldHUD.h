// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
#include "Sound/SoundBase.h"
#include "WorldHUD.generated.h"

class ULootWindowWidget;
class UInventoryWindowWidget;
class UPlayerInfoWindowWidget;
class UUserWidget;
class ALootCorpseActor;

UCLASS()
class PCUBE_API AWorldHUD : public ABaseHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	// --- 루팅 ---
	void ShowLootWindow(ALootCorpseActor* Corpse); // 시체 대상 루팅 UI 표시
	void HideLootWindow(); // 루팅 UI 숨김 + 입력 복구
	
	// 루팅 가능 범위 진입/퇴장 시 프롬프트 이미지 표시/숨김
	void ShowLootPrompt();
	void HideLootPrompt();
	
	// --- 인벤토리 / 플레이어 정보창 ---
	void ToggleInventory(); // I
	void TogglePlayerInfo(); // Tab
	
	void HideInventory();
	void HidePlayerInfo();
	
	// ESC 처리용 - 열려있는 UI 닫기
	bool CloseAnyOpenPanel(); // 하나라도 열려있으면 닫고 true
	
	void ApplyInputMode_GameOnly(); // 월드 조작 모드
	void ApplyInputMode_GameAndUI(UUserWidget* FocusWidget); // UI 조작 모드
	void HideAllPanels(); // UI 간의 충돌 방지
	void SetWorldInputBlocked(bool bBlocked);
	
public:
	// 레벨 BGM (에디터 BP_WorldHUD 에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio")
	TObjectPtr<USoundBase> LevelBGMSound = nullptr;

	// Tab/I/E 키 힌트 위젯 - 항상 화면에 표시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UUserWidget> KeyHintWidgetClass = nullptr;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<ULootWindowWidget> LootWindowClass;
	
	// 루팅 가능 범위 프롬프트 위젯 (E 버튼 이미지 등)
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> LootPromptWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UInventoryWindowWidget> InventoryWindowClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UPlayerInfoWindowWidget> PlayerInfoWindowClass;
	
private:
	UPROPERTY()
	TObjectPtr<UUserWidget> KeyHintWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<ULootWindowWidget> LootWindow = nullptr; // 루팅 창 인스턴스
	
	UPROPERTY()
	TObjectPtr<UUserWidget> LootPromptWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UInventoryWindowWidget> InventoryWindow = nullptr; // 인벤토리 창 인스턴스
	
	UPROPERTY()
	TObjectPtr<UPlayerInfoWindowWidget> PlayerInfoWindow = nullptr; // 플레이어 정보 창 인스턴스
};
