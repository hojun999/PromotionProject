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
	
	void ShowLootWindow(ALootCorpseActor* Corpse);
	void HideLootWindow();
	
	void ToggleInventory();
	void HideInventory();
	
	void TogglePlayerInfo();
	void HidePlayerInfo();
	
	bool CloseAnyOpenPanel();
	
private:
	void ApplyInputMode_GameOnly();
	void ApplyInputMode_GameAndUI(UUserWidget* FocusWidget);
	void HideAllPanels();
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
	TObjectPtr<ULootWindowWidget> LootWindow = nullptr;
	
	UPROPERTY()
	TObjectPtr<UInventoryWindowWidget> InventoryWindow = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPlayerInfoWindowWidget> PlayerInfoWindow = nullptr;
};
