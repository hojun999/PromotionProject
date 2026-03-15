// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHUD.h"
#include "LootCorpseActor.h"
#include "LootWindowWidget.h"
#include "InventoryWindowWidget.h"
#include "PlayerInfoWindowWidget.h"
#include "Blueprint/UserWidget.h"
#include  "AudioManagerSubsystem.h"
#include  "Kismet/GameplayStatics.h"

void AWorldHUD::BeginPlay()
{
	Super::BeginPlay();

	if (LevelBGMSound)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UAudioManagerSubsystem* AM = GI->GetSubsystem<UAudioManagerSubsystem>())
			{
				AM->PlayBGM(LevelBGMSound);
			}
		}
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;
	
	// 루팅 프롬프트 위젯 생성 // 추가됨
	if (LootPromptWidgetClass)
	{
		LootPromptWidget = CreateWidget<UUserWidget>(PC, LootPromptWidgetClass);
		if (LootPromptWidget)
		{
			LootPromptWidget->AddToViewport(60); // LootWindow보다 아래 ZOrder
			LootPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
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
	
	// 키 힌트 위젯 생성 - 항상 화면에 표시 // 추가됨
	if (KeyHintWidgetClass)
	{
		KeyHintWidget = CreateWidget<UUserWidget>(PC, KeyHintWidgetClass);
		if (KeyHintWidget)
		{
			KeyHintWidget->AddToViewport(10);
			KeyHintWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
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
	
	if (UInventoryWindowWidget* IW = Cast<UInventoryWindowWidget>(InventoryWindow))
	{
		IW->Close();
	}
	else if (InventoryWindow)
	{
		InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (UPlayerInfoWindowWidget* PIW = Cast<UPlayerInfoWindowWidget>(PlayerInfoWindow))
	{
		PIW->Close();
	}
	else if (PlayerInfoWindow)
	{
		PlayerInfoWindow->SetVisibility(ESlateVisibility::Collapsed);
	}
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

void AWorldHUD::ShowLootPrompt()
{
	if (LootPromptWidget)
	{
		LootPromptWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AWorldHUD::HideLootPrompt()
{
	if (LootPromptWidget)
	{
		LootPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}


void AWorldHUD::ToggleInventory()
{
	if (!InventoryWindow) return;

	const bool bOpen = (InventoryWindow->GetVisibility() != ESlateVisibility::Visible);
	HideAllPanels();
	
	if (bOpen)
	{
		if (UInventoryWindowWidget* IW = Cast<UInventoryWindowWidget>(InventoryWindow))
		{
			IW->Open();
		}
		else
		{
			InventoryWindow->SetVisibility(ESlateVisibility::Visible);
		}
		ApplyInputMode_GameAndUI(InventoryWindow);
		SetWorldInputBlocked(true);
	}
	else
	{
		if (UInventoryWindowWidget* IW = Cast<UInventoryWindowWidget>(InventoryWindow))
		{
			IW->Close();
		}
		else
		{
			InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
		ApplyInputMode_GameOnly();
		SetWorldInputBlocked(false);
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
		if (UInventoryWindowWidget* IW = Cast<UInventoryWindowWidget>(InventoryWindow))
		{
			IW->Close();
		}
		else
		{
			InventoryWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
		ApplyInputMode_GameOnly();
		SetWorldInputBlocked(false);
		return true;
	}
	
	if (PlayerInfoWindow && PlayerInfoWindow->GetVisibility() == ESlateVisibility::Visible)
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
