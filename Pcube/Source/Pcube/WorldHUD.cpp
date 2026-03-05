// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHUD.h"
#include "LootCorpseActor.h"
#include "LootWindowWidget.h"
#include "PlayerInfoWindowWidget.h"
#include "Blueprint/UserWidget.h"

void AWorldHUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;
	
	//루팅 창 생성
	if (LootWindowClass)
	{
		LootWindow = CreateWidget<ULootWindowWidget>(PC, LootWindowClass);
		if (LootWindow)
		{
			LootWindow->AddToViewport(50);
			LootWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// 인벤토리 창 생성
	if (InventoryWindowClass)
	{
		InventoryWindow = CreateWidget<UUserWidget>(PC, InventoryWindowClass);
		if (InventoryWindow)
		{
			InventoryWindow->AddToViewport(40);
			InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// 플레이어 정보창
	if (PlayerInfoWindowClass)
	{
		PlayerInfoWindow = CreateWidget<UUserWidget>(PC, PlayerInfoWindowClass);
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

	// 다른 패널 닫고 루팅창 열기
	HideAllPanels();

	LootWindow->OpenForCorpse(Corpse);
	LootWindow->SetVisibility(ESlateVisibility::Visible);
	ApplyInputMode_GameAndUI(LootWindow);
	
	// 움직임 막기
	SetWorldInputBlocked(true);
}

void AWorldHUD::HideLootWindow()
{
	if (LootWindow)
	{
		LootWindow->SetVisibility(ESlateVisibility::Collapsed);
	}
	ApplyInputMode_GameOnly();
	
	// 움직임 풀기
	SetWorldInputBlocked(false);
}

void AWorldHUD::ToggleInventory()
{
	if (!InventoryWindow) return;

	const bool bOpen = (InventoryWindow->GetVisibility() != ESlateVisibility::Visible);

	HideAllPanels();

	if (bOpen)
	{
		InventoryWindow->SetVisibility(ESlateVisibility::Visible);
		ApplyInputMode_GameAndUI(InventoryWindow);
		SetWorldInputBlocked(true);
	}
	else
	{
		InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
		ApplyInputMode_GameOnly();
		SetWorldInputBlocked(false);
	}
}

void AWorldHUD::TogglePlayerInfo()
{
	if (!PlayerInfoWindow) return;

	const bool bOpen = (PlayerInfoWindow->GetVisibility() != ESlateVisibility::Visible);

	HideAllPanels();

	if (bOpen)
	{
		if (UPlayerInfoWindowWidget* PIW = Cast<UPlayerInfoWindowWidget>(PlayerInfoWindow))
		{
			PIW->Open();
		}
		else
		{
			PlayerInfoWindow->SetVisibility(ESlateVisibility::Visible);
		}
		ApplyInputMode_GameAndUI(PlayerInfoWindow);
		SetWorldInputBlocked(true);
	}
	else
	{
		if (UPlayerInfoWindowWidget* PIW = Cast<UPlayerInfoWindowWidget>(PlayerInfoWindow))
		{
			PIW->Close();
		}
		else
		{
			PlayerInfoWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
		ApplyInputMode_GameOnly();
		SetWorldInputBlocked(false);
	}
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
		InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
		ApplyInputMode_GameOnly();
		SetWorldInputBlocked(false);
		return true;
	}
	
	if (PlayerInfoWindow && PlayerInfoWindow->GetVisibility() == ESlateVisibility::Visible)
	{
		PlayerInfoWindow->SetVisibility(ESlateVisibility::Collapsed);
		ApplyInputMode_GameOnly();
		SetWorldInputBlocked(false);
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
