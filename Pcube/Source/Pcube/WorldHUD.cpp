// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHUD.h"
#include "InventoryWindowWidget.h"
#include "LootCorpseActor.h"
#include "LootWindowWidget.h"
#include "PlayerInfoWindowWidget.h"

void AWorldHUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;
	
	if (LootWindowClass)
	{
		LootWindow = CreateWidget<ULootWindowWidget>(PC, LootWindowClass);
		if (LootWindow)
		{
			LootWindow->AddToViewport(50);
			LootWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (InventoryWindowClass)
	{
		InventoryWindow = CreateWidget<UInventoryWindowWidget>(PC, InventoryWindowClass);
		if (InventoryWindow)
		{
			InventoryWindow->AddToViewport(40);
			InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (PlayerInfoWindowClass)
	{
		PlayerInfoWindow = CreateWidget<UPlayerInfoWindowWidget>(PC, PlayerInfoWindowClass);
		if (PlayerInfoWindow)
		{
			PlayerInfoWindow->AddToViewport(40);
			PlayerInfoWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void AWorldHUD::ApplyInputMode_GameOnly()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = false;
	}
}

void AWorldHUD::ApplyInputMode_GameAndUI(UUserWidget* FocusWidget)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameAndUI Mode;
		if (FocusWidget)
		{
			Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
		}
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
}

void AWorldHUD::HideAllPanels()
{
	if (LootWindow) LootWindow->SetVisibility(ESlateVisibility::Collapsed);
	if (InventoryWindow) InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
	if (PlayerInfoWindow) PlayerInfoWindow->SetVisibility(ESlateVisibility::Collapsed);
}

void AWorldHUD::ShowLootWindow(ALootCorpseActor* Corpse)
{
	if (!LootWindow || !IsValid(Corpse)) return;

	HideAllPanels();

	LootWindow->OpenForCorpse(Corpse);
	LootWindow->SetVisibility(ESlateVisibility::Visible);
	ApplyInputMode_GameAndUI(LootWindow);
	SetWorldInputBlocked(true);
}

void AWorldHUD::HideLootWindow()
{
	if (LootWindow)
	{
		LootWindow->SetVisibility(ESlateVisibility::Collapsed);
	}
	ApplyInputMode_GameOnly();
	SetWorldInputBlocked(false);
}

void AWorldHUD::ToggleInventory()
{
	if (!InventoryWindow) return;

	const bool bOpen = (InventoryWindow->GetVisibility() != ESlateVisibility::Visible);
	if (bOpen)
	{
		HideAllPanels();
		InventoryWindow->Open();
		ApplyInputMode_GameAndUI(InventoryWindow);
		SetWorldInputBlocked(true);
	}
	else
	{
		HideInventory();
	}
}

void AWorldHUD::HideInventory()
{
	if (InventoryWindow)
	{
		InventoryWindow->Close();
	}
	ApplyInputMode_GameOnly();
	SetWorldInputBlocked(false);
}

void AWorldHUD::TogglePlayerInfo()
{
	if (!PlayerInfoWindow) return;

	const bool bOpen = (PlayerInfoWindow->GetVisibility() != ESlateVisibility::Visible);
	if (bOpen)
	{
		HideAllPanels();
		PlayerInfoWindow->Open();
		ApplyInputMode_GameAndUI(PlayerInfoWindow);
		SetWorldInputBlocked(true);
	}
	else
	{
		HidePlayerInfo();
	}
}

void AWorldHUD::HidePlayerInfo()
{
	if (PlayerInfoWindow)
	{
		PlayerInfoWindow->Close();
	}
	ApplyInputMode_GameOnly();
	SetWorldInputBlocked(false);
}

bool AWorldHUD::CloseAnyOpenPanel()
{
	if (LootWindow && LootWindow->GetVisibility() == ESlateVisibility::Visible)
	{
		HideLootWindow();
		return true;
	}
	
	if (InventoryWindow && InventoryWindow->GetVisibility() == ESlateVisibility::Visible)
	{
		HideInventory();
		return true;
	}
	
	if (PlayerInfoWindow && PlayerInfoWindow->GetVisibility() == ESlateVisibility::Visible)
	{
		HidePlayerInfo();
		return true;
	}
	return false;
}

void AWorldHUD::SetWorldInputBlocked(bool bBlocked)
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetIgnoreMoveInput(bBlocked);
		PC->SetIgnoreLookInput(bBlocked);
	}
}
