// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
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
	
private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<ULootWindowWidget> LootWindowClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UInventoryWindowWidget> InventoryWindowClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UPlayerInfoWindowWidget> PlayerInfoWindowClass;
	
private:
	UPROPERTY()
	TObjectPtr<ULootWindowWidget> LootWindow = nullptr; // 루팅 창 인스턴스
	
	UPROPERTY()
	TObjectPtr<UInventoryWindowWidget> InventoryWindow = nullptr; // 인벤토리 창 인스턴스
	
	UPROPERTY()
	TObjectPtr<UPlayerInfoWindowWidget> PlayerInfoWindow = nullptr; // 플레이어 정보 창 인스턴스
};
