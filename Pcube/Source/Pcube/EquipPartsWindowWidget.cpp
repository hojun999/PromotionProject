#include "EquipPartsWindowWidget.h"

#include "ArmorDataAsset.h"
#include "EquipPartsSlotWidget.h"
#include "EquipSlotWidget.h"
#include "EquipmentSubsystem.h"
#include "InventorySubsystem.h"
#include "WeaponPartDataAsset.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UEquipPartsWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		EquipmentSubsystem = GI->GetSubsystem<UEquipmentSubsystem>();
		InventorySubsystem = GI->GetSubsystem<UInventorySubsystem>();
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleCloseClicked);
		Btn_Close->OnClicked.AddDynamic(this, &UEquipPartsWindowWidget::HandleCloseClicked);
	}

	if (InventorySubsystem)
	{
		InventorySubsystem->OnInventoryChanged.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleInventoryChanged);
		InventorySubsystem->OnInventoryChanged.AddDynamic(this, &UEquipPartsWindowWidget::HandleInventoryChanged);
	}

	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleEquipmentChanged);
		EquipmentSubsystem->OnEquipmentChanged.AddDynamic(this, &UEquipPartsWindowWidget::HandleEquipmentChanged);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UEquipPartsWindowWidget::NativeDestruct()
{
	if (InventorySubsystem)
	{
		InventorySubsystem->OnInventoryChanged.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleInventoryChanged);
	}

	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleEquipmentChanged);
	}

	Super::NativeDestruct();
}

void UEquipPartsWindowWidget::ResetActiveEquipment()
{
	ActiveWeapon = nullptr;
	ActiveArmor = nullptr;
	EquipmentButtonKey = NAME_None;
	SelectedPartSlotKey = NAME_None;
	PartyIndex = INDEX_NONE;
}

void UEquipPartsWindowWidget::OpenForWeapon(int32 InPartyIndex, FName InEquipmentButtonKey, UWeaponDataAsset* InWeapon)
{
	ResetActiveEquipment();
	PartyIndex = InPartyIndex;
	EquipmentButtonKey = InEquipmentButtonKey;
	CurrentEquipmentKind = EPlayerInfoEquipmentKind::Weapon;
	ActiveWeapon = InWeapon;
	AutoSelectFirstSlotIfNeeded();
	SetVisibility(ESlateVisibility::Visible);
	Refresh();
}

void UEquipPartsWindowWidget::OpenForArmor(int32 InPartyIndex, FName InEquipmentButtonKey, UArmorDataAsset* InArmor)
{
	ResetActiveEquipment();
	PartyIndex = InPartyIndex;
	EquipmentButtonKey = InEquipmentButtonKey;
	CurrentEquipmentKind = EPlayerInfoEquipmentKind::Armor;
	ActiveArmor = InArmor;
	AutoSelectFirstSlotIfNeeded();
	SetVisibility(ESlateVisibility::Visible);
	Refresh();
}


// void UEquipPartsWindowWidget::OpenForEquipment(int32 InPartyIndex, FName InEquipmentKey)
// {
// 	PartyIndex = InPartyIndex;
// 	EquipmentKey = InEquipmentKey;
// 	SetVisibility(ESlateVisibility::Visible);
// 	Refresh();
// }

void UEquipPartsWindowWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
	if (Canvas_Slots)
	{
		Canvas_Slots->ClearChildren();
	}
	if (Grid_Parts)
	{
		Grid_Parts->ClearChildren();
	}
	ResetActiveEquipment();
}

const TArray<FWeaponModSlotDef>* UEquipPartsWindowWidget::GetActiveSlots() const
{
	if (CurrentEquipmentKind == EPlayerInfoEquipmentKind::Weapon && ActiveWeapon)
	{
		return &ActiveWeapon->ModSlots;
	}
	if (CurrentEquipmentKind == EPlayerInfoEquipmentKind::Armor && ActiveArmor)
	{
		return &ActiveArmor->ModSlots;
	}
	return nullptr;
}

UTexture2D* UEquipPartsWindowWidget::GetActiveIllustration() const
{
	if (CurrentEquipmentKind == EPlayerInfoEquipmentKind::Weapon && ActiveWeapon)
	{
		return ActiveWeapon->IllustrationTexture ? ActiveWeapon->IllustrationTexture : ActiveWeapon->Icon;
	}
	if (CurrentEquipmentKind == EPlayerInfoEquipmentKind::Armor && ActiveArmor)
	{
		return ActiveArmor->IllustrationTexture ? ActiveArmor->IllustrationTexture : ActiveArmor->Icon;
	}
	return nullptr;
}

FText UEquipPartsWindowWidget::GetActiveTitle() const
{
	if (CurrentEquipmentKind == EPlayerInfoEquipmentKind::Weapon && ActiveWeapon)
	{
		return !ActiveWeapon->DisplayName.IsEmpty() ? ActiveWeapon->DisplayName : FText::FromName(EquipmentButtonKey);
	}
	if (CurrentEquipmentKind == EPlayerInfoEquipmentKind::Armor && ActiveArmor)
	{
		return !ActiveArmor->DisplayName.IsEmpty() ? ActiveArmor->DisplayName : FText::FromName(EquipmentButtonKey);
	}
	return FText::FromName(EquipmentButtonKey);
}

void UEquipPartsWindowWidget::AutoSelectFirstSlotIfNeeded()
{
	if (SelectedPartSlotKey != NAME_None)
	{
		return;
	}

	if (const TArray<FWeaponModSlotDef>* Slots = GetActiveSlots())
	{
		if (Slots->Num() > 0)
		{
			SelectedPartSlotKey = (*Slots)[0].SlotId;
		}
	}
}

void UEquipPartsWindowWidget::Refresh()
{
	if (GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}

	AutoSelectFirstSlotIfNeeded();

	if (Text_Title)
	{
		Text_Title->SetText(GetActiveTitle());
	}

	if (Img_EquipmentIllustration)
	{
		if (UTexture2D* Illustration = GetActiveIllustration())
		{
			Img_EquipmentIllustration->SetBrushFromTexture(Illustration);
			Img_EquipmentIllustration->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_EquipmentIllustration->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	RebuildSlotCanvas();
	RebuildCandidateGrid();
}

void UEquipPartsWindowWidget::HandleCloseClicked()
{
	Close();
}

void UEquipPartsWindowWidget::HandleInventoryChanged()
{
	Refresh();
}

void UEquipPartsWindowWidget::HandleEquipmentChanged(int32 ChangedPartyIndex)
{
	if (ChangedPartyIndex == PartyIndex)
	{
		Refresh();
	}
}

void UEquipPartsWindowWidget::HandlePartSlotSelected(FName PartSlotKey)
{
	if (PartSlotKey == NAME_None) return;
	SelectedPartSlotKey = PartSlotKey;
	Refresh();
}

void UEquipPartsWindowWidget::HandlePartSlotUnequip(FName PartSlotKey)
{
	if (!EquipmentSubsystem || PartSlotKey == NAME_None || PartyIndex == INDEX_NONE)
	{
		return;
	}

	if (EquipmentSubsystem->UnequipWeaponPart(PartyIndex, PartSlotKey))
	{
		SelectedPartSlotKey = PartSlotKey;
		Refresh();
	}
}

void UEquipPartsWindowWidget::HandlePartCandidateClicked(UWeaponPartDataAsset* Part)
{
	if (!EquipmentSubsystem || !Part || PartyIndex == INDEX_NONE || SelectedPartSlotKey == NAME_None)
	{
		return;
	}

	if (EquipmentSubsystem->EquipWeaponPart(PartyIndex, SelectedPartSlotKey, Part))
	{
		Refresh();
	}
}

void UEquipPartsWindowWidget::RebuildSlotCanvas()
{
	if (!Canvas_Slots)
	{
		return;
	}

	Canvas_Slots->ClearChildren();

	if (!EquipSlotWidgetClass || !EquipmentSubsystem || PartyIndex == INDEX_NONE)
	{
		return;
	}

	const TArray<FWeaponModSlotDef>* Slots = GetActiveSlots();
	if (!Slots)
	{
		return;
	}

	for (const FWeaponModSlotDef& Def : *Slots)
	{
		if (Def.SlotId == NAME_None)
		{
			continue;
		}

		UEquipSlotWidget* SlotWidget = CreateWidget<UEquipSlotWidget>(GetOwningPlayer(), EquipSlotWidgetClass);
		if (!SlotWidget) continue;

		SlotWidget->InitSlot(Def.SlotId, Def.DisplayName);
		SlotWidget->SetEquipped(EquipmentSubsystem->GetEquippedPart(PartyIndex, Def.SlotId));
		SlotWidget->SetRenderOpacity(SelectedPartSlotKey == Def.SlotId ? 1.f : 0.6f);
		SlotWidget->OnSelected.RemoveDynamic(this, &UEquipPartsWindowWidget::HandlePartSlotSelected);
		SlotWidget->OnSelected.AddDynamic(this, &UEquipPartsWindowWidget::HandlePartSlotSelected);
		SlotWidget->OnUnequip.RemoveDynamic(this, &UEquipPartsWindowWidget::HandlePartSlotUnequip);
		SlotWidget->OnUnequip.AddDynamic(this, &UEquipPartsWindowWidget::HandlePartSlotUnequip);

		if (UCanvasPanelSlot* CanvasSlot = Canvas_Slots->AddChildToCanvas(SlotWidget))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(Def.UIAnchor.X, Def.UIAnchor.Y, Def.UIAnchor.X, Def.UIAnchor.Y));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(Def.UIPixelOffset);
			CanvasSlot->SetSize(SlotWidgetSize);
		}
	}
}

void UEquipPartsWindowWidget::RebuildCandidateGrid()
{
	if (!Grid_Parts)
	{
		return;
	}

	Grid_Parts->ClearChildren();

	if (!InventorySubsystem || !EquipmentSubsystem || !PartCandidateWidgetClass || PartyIndex == INDEX_NONE || SelectedPartSlotKey == NAME_None)
	{
		return;
	}

	TMap<TObjectPtr<UWeaponPartDataAsset>, int32> CompatibleParts;
	for (const FInventoryStack& Stack : InventorySubsystem->GetAllStacks())
	{
		UWeaponPartDataAsset* Part = Cast<UWeaponPartDataAsset>(Stack.Item);
		if (!Part || Stack.Quantity <= 0)
		{
			continue;
		}

		if (!EquipmentSubsystem->CanEquipWeaponPart(PartyIndex, SelectedPartSlotKey, Part))
		{
			continue;
		}

		CompatibleParts.FindOrAdd(Part) += Stack.Quantity;
	}

	TArray<TPair<TObjectPtr<UWeaponPartDataAsset>, int32>> Entries;
	for (const TPair<TObjectPtr<UWeaponPartDataAsset>, int32>& Pair : CompatibleParts)
	{
		Entries.Add(Pair);
	}

	Entries.Sort([](const TPair<TObjectPtr<UWeaponPartDataAsset>, int32>& A, const TPair<TObjectPtr<UWeaponPartDataAsset>, int32>& B)
	{
		const FString NameA = A.Key ? A.Key->DisplayName.ToString() : FString();
		const FString NameB = B.Key ? B.Key->DisplayName.ToString() : FString();
		return NameA < NameB;
	});

	const int32 Count = FMath::Min(MaxSlots, Entries.Num());
	for (int32 Index = 0; Index < Count; ++Index)
	{
		UEquipPartsSlotWidget* CandidateWidget = CreateWidget<UEquipPartsSlotWidget>(GetOwningPlayer(), PartCandidateWidgetClass);
		if (!CandidateWidget) continue;

		CandidateWidget->Init(Entries[Index].Key, Entries[Index].Value);
		CandidateWidget->OnClicked.RemoveDynamic(this, &UEquipPartsWindowWidget::HandlePartCandidateClicked);
		CandidateWidget->OnClicked.AddDynamic(this, &UEquipPartsWindowWidget::HandlePartCandidateClicked);

		if (UUniformGridSlot* GridSlot = Grid_Parts->AddChildToUniformGrid(CandidateWidget, Index / Cols, Index % Cols))
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}


// void UEquipPartsWindowWidget::HandlePartSlotSelected(UWeaponPartDataAsset* Part)
// {
// 	if (!EquipmentSubsystem || !Part || PartyIndex == INDEX_NONE || EquipmentKey == NAME_None)
// 	{
// 		return;
// 	}
//
// 	if (EquipmentSubsystem->EquipWeaponPart(PartyIndex, EquipmentKey, Part))
// 	{
// 		Refresh();
// 	}
// }

// void UEquipPartsWindowWidget::RebuildGrid()
// {
// 	if (!Grid_Parts)
// 	{
// 		return;
// 	}
//
// 	Grid_Parts->ClearChildren();
//
// 	if (!InventorySubsystem || !EquipmentSubsystem || !PartCandidateWidgetClass || PartyIndex == INDEX_NONE || EquipmentKey == NAME_None)
// 	{
// 		return;
// 	}
//
// 	TMap<TObjectPtr<UWeaponPartDataAsset>, int32> CompatibleParts;
//
// 	for (const FInventoryStack& Stack : InventorySubsystem->GetAllStacks())
// 	{
// 		UWeaponPartDataAsset* Part = Cast<UWeaponPartDataAsset>(Stack.Item);
// 		if (!Part || Stack.Quantity <= 0)
// 		{
// 			continue;
// 		}
//
// 		if (!EquipmentSubsystem->CanEquipWeaponPart(PartyIndex, EquipmentKey, Part))
// 		{
// 			continue;
// 		}
//
// 		CompatibleParts.FindOrAdd(Part) += Stack.Quantity;
// 	}
//
// 	TArray<TPair<TObjectPtr<UWeaponPartDataAsset>, int32>> Entries;
// 	for (const TPair<TObjectPtr<UWeaponPartDataAsset>, int32>& It : CompatibleParts)
// 	{
// 		Entries.Add(It);
// 	}
//
// 	Entries.Sort([](const TPair<TObjectPtr<UWeaponPartDataAsset>, int32>& A, const TPair<TObjectPtr<UWeaponPartDataAsset>, int32>& B)
// 	{
// 		const FString NameA = A.Key ? A.Key->DisplayName.ToString() : FString();
// 		const FString NameB = B.Key ? B.Key->DisplayName.ToString() : FString();
// 		return NameA < NameB;
// 	});
//
// 	const int32 Count = FMath::Min(MaxSlots, Entries.Num());
// 	for (int32 i = 0; i < Count; ++i)
// 	{
// 		UEquipPartsSlotWidget* PartsSlot = CreateWidget<UEquipPartsSlotWidget>(GetOwningPlayer(), PartCandidateWidgetClass);
// 		if (!PartsSlot) continue;
//
// 		PartsSlot->Init(Entries[i].Key, Entries[i].Value);
// 		PartsSlot->OnClicked.RemoveDynamic(this, &UEquipPartsWindowWidget::HandlePartSlotSelected);
// 		PartsSlot->OnClicked.AddDynamic(this, &UEquipPartsWindowWidget::HandlePartSlotSelected);
//
// 		if (UUniformGridSlot* GridSlot = Grid_Parts->AddChildToUniformGrid(PartsSlot, i / Cols, i % Cols))
// 		{
// 			GridSlot->SetHorizontalAlignment(HAlign_Fill);
// 			GridSlot->SetVerticalAlignment(VAlign_Fill);
// 		}
// 	}
// }
