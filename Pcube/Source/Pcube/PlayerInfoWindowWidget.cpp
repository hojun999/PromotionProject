// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInfoWindowWidget.h"

#include "PlayerInfoEquipButtonWidget.h"
#include "EquipPartsWindowWidget.h"

#include "EquipmentSubsystem.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"
#include "WeaponDataAsset.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UPlayerInfoWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		EquipmentSubsystem = GI->GetSubsystem<UEquipmentSubsystem>();
		TransferSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	}

	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
		EquipmentSubsystem->OnEquipmentChanged.AddDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
		EquipmentSubsystem->EnsurePartySize(GetPartyCount());
	}

	EnsureDefaultWeaponsInitialized();

	if (Btn_Ally0)
	{
		Btn_Ally0->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleAlly0Clicked);
		Btn_Ally0->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleAlly0Clicked);
	}
	if (Btn_Ally1)
	{
		Btn_Ally1->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleAlly1Clicked);
		Btn_Ally1->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleAlly1Clicked);
	}

	if (Btn_WeaponSlot)
	{
		Btn_WeaponSlot->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleWeaponSlotClicked);
		Btn_WeaponSlot->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleWeaponSlotClicked);
	}
	if (Btn_ArmorSlot)
	{
		Btn_ArmorSlot->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleArmorSlotClicked);
		Btn_ArmorSlot->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleArmorSlotClicked);
	}

	if (EquipPartsWindowClass && GetOwningPlayer())
	{
		EquipPartsWindow = CreateWidget<UEquipPartsWindowWidget>(GetOwningPlayer(), EquipPartsWindowClass);
		if (EquipPartsWindow)
		{
			EquipPartsWindow->AddToViewport(55);
			EquipPartsWindow->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	RefreshPortraitButtons();
	SelectPartyMember(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::NativeDestruct()
{
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentChanged);
	}

	Super::NativeDestruct();
}

void UPlayerInfoWindowWidget::Open()
{
	EnsureDefaultWeaponsInitialized();
	SetVisibility(ESlateVisibility::Visible);
	RefreshPortraitButtons();
	SelectPartyMember(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::Close()
{
	HideEquipPartsWindow();
	SetVisibility(ESlateVisibility::Collapsed);
}

int32 UPlayerInfoWindowWidget::GetPartyCount() const
{
	if (!TransferSubsystem) return 0;
	return TransferSubsystem->BattleInfo.AlliesToSpawn.Num();
}

UUnitDataAsset* UPlayerInfoWindowWidget::GetPartyUnitData(int32 PartyIndex) const
{
	if (!TransferSubsystem) return nullptr;
	if (!TransferSubsystem->BattleInfo.AlliesToSpawn.IsValidIndex(PartyIndex)) return nullptr;

	const TSoftObjectPtr<UUnitDataAsset> Soft = TransferSubsystem->BattleInfo.AlliesToSpawn[PartyIndex].UnitDataAsset;
	if (Soft.IsNull()) return nullptr;

	return Soft.LoadSynchronous();
}

void UPlayerInfoWindowWidget::EnsureDefaultWeaponsInitialized()
{
	if (!EquipmentSubsystem) return;

	const int32 PartyCount = GetPartyCount();
	EquipmentSubsystem->EnsurePartySize(PartyCount);

	for (int32 PartyIndex = 0; PartyIndex < PartyCount; ++PartyIndex)
	{
		if (EquipmentSubsystem->GetEquippedWeapon(PartyIndex))
		{
			continue;
		}

		if (UUnitDataAsset* UnitDA = GetPartyUnitData(PartyIndex))
		{
			if (UnitDA->DefaultWeapon)
			{
				EquipmentSubsystem->EquipWeapon(PartyIndex, UnitDA->DefaultWeapon);
			}
		}
	}
}

void UPlayerInfoWindowWidget::HandleAlly0Clicked()
{
	SelectPartyMember(0);
}

void UPlayerInfoWindowWidget::HandleAlly1Clicked()
{
	SelectPartyMember(1);
}

void UPlayerInfoWindowWidget::HandleWeaponSlotClicked()
{
	BP_OpenWeaponModWindow(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::HandleArmorSlotClicked()
{
	BP_OpenArmorModWindow(SelectedPartyIndex);
}

void UPlayerInfoWindowWidget::HandleEquipmentButtonClicked(FName EquipmentKey)
{
	if (TryToggleEquipmentPartsWindow(EquipmentKey))
	{
		return;
	}

	BP_OpenEquipmentPartsSelectionWindow(SelectedPartyIndex, EquipmentKey);
}

void UPlayerInfoWindowWidget::HandleEquipmentChanged(int32 PartyIndex)
{
	if (PartyIndex != SelectedPartyIndex) return;
	RefreshSelectedMemberPanel();
}

void UPlayerInfoWindowWidget::SelectPartyMember(int32 PartyIndex)
{
	HideEquipPartsWindow();

	const int32 N = GetPartyCount();
	if (N <= 0)
	{
		SelectedPartyIndex = 0;
		RefreshPortraitButtons();
		RefreshSelectedMemberPanel();
		return;
	}

	SelectedPartyIndex = FMath::Clamp(PartyIndex, 0, N - 1);
	RefreshPortraitButtons();
	RefreshSelectedMemberPanel();
}

void UPlayerInfoWindowWidget::RefreshPortraitButtons()
{
	const int32 N = GetPartyCount();

	if (Btn_Ally0)
	{
		Btn_Ally0->SetVisibility(N >= 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Btn_Ally0->SetIsEnabled(N >= 1);
	}
	if (Img_Portrait0)
	{
		if (UUnitDataAsset* DA0 = GetPartyUnitData(0))
		{
			if (DA0->PortraitTexture) Img_Portrait0->SetBrushFromTexture(DA0->PortraitTexture);
			else if (DA0->UnitIcon) Img_Portrait0->SetBrushFromTexture(DA0->UnitIcon);
		}
		Img_Portrait0->SetRenderOpacity(SelectedPartyIndex == 0 ? 1.f : 0.35f);
	}

	if (Btn_Ally1)
	{
		Btn_Ally1->SetVisibility(N >= 2 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Btn_Ally1->SetIsEnabled(N >= 2);
	}
	if (Img_Portrait1)
	{
		if (UUnitDataAsset* DA1 = GetPartyUnitData(1))
		{
			if (DA1->PortraitTexture) Img_Portrait1->SetBrushFromTexture(DA1->PortraitTexture);
			else if (DA1->UnitIcon) Img_Portrait1->SetBrushFromTexture(DA1->UnitIcon);
		}
		Img_Portrait1->SetRenderOpacity(SelectedPartyIndex == 1 ? 1.f : 0.35f);
	}
}

void UPlayerInfoWindowWidget::RefreshSelectedMemberPanel()
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);

	if (Img_Illustration)
	{
		if (UnitDA)
		{
			if (UnitDA->UnitIcon) Img_Illustration->SetBrushFromTexture(UnitDA->UnitIcon);
			else if (UnitDA->PortraitTexture) Img_Illustration->SetBrushFromTexture(UnitDA->PortraitTexture);
		}
	}

	if (Img_WeaponIcon)
	{
		UTexture2D* WeaponIcon = nullptr;
		if (EquipmentSubsystem)
		{
			if (UWeaponDataAsset* Weapon = EquipmentSubsystem->GetEquippedWeapon(SelectedPartyIndex))
			{
				WeaponIcon = Weapon->Icon;
			}
		}
		if (!WeaponIcon && UnitDA && UnitDA->DefaultWeapon)
		{
			WeaponIcon = UnitDA->DefaultWeapon->Icon;
		}

		if (WeaponIcon)
		{
			Img_WeaponIcon->SetBrushFromTexture(WeaponIcon);
			Img_WeaponIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_WeaponIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (Img_ArmorIcon)
	{
		Img_ArmorIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	RebuildEquipmentButtons();
	RefreshStatsPanel();
}

bool UPlayerInfoWindowWidget::ResolveEquipmentButtonDef(FName EquipmentKey, FPlayerInfoEquipmentButtonDef& OutDef) const
{
	if (UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex))
	{
		for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
		{
			if (Def.EquipmentKey == EquipmentKey)
			{
				OutDef = Def;
				return true;
			}
		}
	}

	return false;
}

FVector2D UPlayerInfoWindowWidget::GetEquipButtonWindowPosition(FName EquipmentKey) const
{
	if (const TObjectPtr<UPlayerInfoEquipButtonWidget>* Found = EquipButtonsByKey.Find(EquipmentKey))
	{
		if (UPlayerInfoEquipButtonWidget* ButtonWidget = Found->Get())
		{
			const FGeometry Geometry = ButtonWidget->GetCachedGeometry();
			FVector2D PixelPosition;
			FVector2D ViewportPosition;
			USlateBlueprintLibrary::LocalToViewport(this, Geometry, FVector2D(Geometry.GetLocalSize().X, 0.f), PixelPosition, ViewportPosition);
			return ViewportPosition + FVector2D(12.f, 0.f);
		}
	}

	return FVector2D(64.f, 64.f);
}

bool UPlayerInfoWindowWidget::TryToggleEquipmentPartsWindow(FName EquipmentKey)
{
	FPlayerInfoEquipmentButtonDef ButtonDef;
	if (!ResolveEquipmentButtonDef(EquipmentKey, ButtonDef))
	{
		return false;
	}

	if (!EquipPartsWindow)
	{
		return false;
	}

	if (EquipPartsWindow->IsOpenFor(SelectedPartyIndex, EquipmentKey))
	{
		HideEquipPartsWindow();
		return true;
	}

	if (EquipPartsWindow->OpenForEquipment(SelectedPartyIndex, EquipmentKey))
	{
		EquipPartsWindow->SetWindowScreenPosition(GetEquipButtonWindowPosition(EquipmentKey));
		return true;
	}

	return false;
}

void UPlayerInfoWindowWidget::HideEquipPartsWindow()
{
	if (EquipPartsWindow)
	{
		EquipPartsWindow->CloseWindow();
	}
}

void UPlayerInfoWindowWidget::RebuildEquipmentButtons()
{
	if (!Canvas_EquipButtons) return;

	Canvas_EquipButtons->ClearChildren();
	EquipButtonsByKey.Empty();

	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);
	if (!UnitDA) return;
	if (!EquipButtonWidgetClass) return;

	for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
	{
		if (Def.EquipmentKey.IsNone()) continue;

		UPlayerInfoEquipButtonWidget* W = CreateWidget<UPlayerInfoEquipButtonWidget>(GetOwningPlayer(), EquipButtonWidgetClass);
		if (!W) continue;

		UTexture2D* Icon = Def.IconOverride;
		if (!Icon && Def.Kind == EPlayerInfoEquipmentKind::Weapon)
		{
			UWeaponDataAsset* Weapon = nullptr;
			if (EquipmentSubsystem)
			{
				Weapon = EquipmentSubsystem->GetEquippedWeapon(SelectedPartyIndex);
			}
			if (!Weapon && Def.WeaponOverride)
			{
				Weapon = Def.WeaponOverride;
			}
			if (!Weapon)
			{
				Weapon = UnitDA->DefaultWeapon;
			}
			if (Weapon)
			{
				Icon = Weapon->Icon;
			}
		}

		W->Init(Def.EquipmentKey, Icon);
		W->OnClicked.RemoveDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentButtonClicked);
		W->OnClicked.AddDynamic(this, &UPlayerInfoWindowWidget::HandleEquipmentButtonClicked);

		UCanvasPanelSlot* CanvasSlot = Canvas_EquipButtons->AddChildToCanvas(W);
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetAnchors(FAnchors(Def.UIAnchor.X, Def.UIAnchor.Y, Def.UIAnchor.X, Def.UIAnchor.Y));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(Def.UIPixelOffset);
		CanvasSlot->SetSize(Def.UISize);

		EquipButtonsByKey.FindOrAdd(Def.EquipmentKey) = W;
	}
}

void UPlayerInfoWindowWidget::RefreshStatsPanel()
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(SelectedPartyIndex);

	const float MaxHP = UnitDA ? UnitDA->BaseStats.MaxHP : 0.f;
	const float SPD = UnitDA ? UnitDA->BaseStats.Speed : 0.f;
	const float ATK = UnitDA ? UnitDA->BaseStats.AttackPower : 0.f;

	float CurHP = MaxHP;
	int32 CurSP = UnitDA ? UnitDA->BaseSkillPoints : 0;

	if (TransferSubsystem)
	{
		const float SavedHP = TransferSubsystem->GetSavedHP(SelectedPartyIndex);
		if (SavedHP >= 0.f) CurHP = SavedHP;

		const int32 SavedSP = TransferSubsystem->GetSavedSP(SelectedPartyIndex);
		if (SavedSP >= 0) CurSP = SavedSP;
	}

	if (Text_HP) Text_HP->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f / %.0f"), CurHP, MaxHP)));
	if (Text_SP) Text_SP->SetText(FText::FromString(FString::Printf(TEXT("SP %d"), CurSP)));
	if (Text_ATK) Text_ATK->SetText(FText::FromString(FString::Printf(TEXT("ATK %.0f"), ATK)));
	if (Text_SPD) Text_SPD->SetText(FText::FromString(FString::Printf(TEXT("SPD %.0f"), SPD)));

	if (EquipmentSubsystem)
	{
		const FWeaponPartEffect E = EquipmentSubsystem->GetTotalWeaponPartEffect(SelectedPartyIndex);

		if (Text_Effects)
		{
			TArray<FString> Lines;
			if (E.BonusProjectileCount != 0) Lines.Add(FString::Printf(TEXT("Proj %+d"), E.BonusProjectileCount));
			if (E.BonusHitCount != 0) Lines.Add(FString::Printf(TEXT("Hit %+d"), E.BonusHitCount));
			if (!FMath::IsNearlyZero(E.DamageMulAdd)) Lines.Add(FString::Printf(TEXT("DmgAdd %+0.2f"), E.DamageMulAdd));
			if (!FMath::IsNearlyEqual(E.DamageMulMul, 1.f)) Lines.Add(FString::Printf(TEXT("DmgMul x%0.2f"), E.DamageMulMul));
			if (Lines.Num() == 0) Lines.Add(TEXT("Effects: None"));

			Text_Effects->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
		}

		if (Text_Proj) Text_Proj->SetText(FText::FromString(FString::Printf(TEXT("Proj %+d"), E.BonusProjectileCount)));
		if (Text_Hit) Text_Hit->SetText(FText::FromString(FString::Printf(TEXT("Hit %+d"), E.BonusHitCount)));
		if (Text_DmgMul) Text_DmgMul->SetText(FText::FromString(FString::Printf(TEXT("DmgAdd %+0.2f / x%.2f"), E.DamageMulAdd, E.DamageMulMul)));
	}
	else
	{
		if (Text_Effects) Text_Effects->SetText(FText::FromString(TEXT("Effects: None")));
		if (Text_Proj) Text_Proj->SetText(FText::FromString(TEXT("Proj +0")));
		if (Text_Hit) Text_Hit->SetText(FText::FromString(TEXT("Hit +0")));
		if (Text_DmgMul) Text_DmgMul->SetText(FText::FromString(TEXT("DmgAdd +0.00 / x1.00")));
	}
}
