#include "EquipPartsWindowWidget.h"

#include "EquipPartsSlotWidget.h"
#include "EquipmentSubsystem.h"
#include "InventorySubsystem.h"
#include "BattleInfoTransferSubsystem.h"
#include "WeaponDataAsset.h"
#include "WeaponPartDataAsset.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UEquipPartsWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		EquipmentSubsystem = GI->GetSubsystem<UEquipmentSubsystem>();
		InventorySubsystem = GI->GetSubsystem<UInventorySubsystem>();
		TransferSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleCloseClicked);
		Btn_Close->OnClicked.AddDynamic(this, &UEquipPartsWindowWidget::HandleCloseClicked);
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
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->OnEquipmentChanged.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleEquipmentChanged);
	}

	Super::NativeDestruct();
}

bool UEquipPartsWindowWidget::OpenForEquipment(int32 InPartyIndex, FName InEquipmentKey)
{
	OpenPartyIndex = InPartyIndex;
	OpenEquipmentKey = InEquipmentKey;
	ResolvedWeapon = nullptr;

	FPlayerInfoEquipmentButtonDef Def;
	if (!ResolveEquipmentButtonDef(Def))
	{
		CloseWindow();
		return false;
	}

	// 현재 코드베이스에서 실동작하는 장비 컨테이너는 Weapon뿐이다.
	if (Def.Kind != EPlayerInfoEquipmentKind::Weapon)
	{
		CloseWindow();
		return false;
	}

	ResolvedWeapon = ResolveTargetWeapon(Def);
	if (!ResolvedWeapon)
	{
		CloseWindow();
		return false;
	}

	SetVisibility(ESlateVisibility::Visible);
	RefreshWindow();
	return true;
}

void UEquipPartsWindowWidget::CloseWindow()
{
	OpenPartyIndex = INDEX_NONE;
	OpenEquipmentKey = NAME_None;
	ResolvedWeapon = nullptr;
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UEquipPartsWindowWidget::IsOpenFor(int32 InPartyIndex, FName InEquipmentKey) const
{
	return GetVisibility() == ESlateVisibility::Visible
		&& OpenPartyIndex == InPartyIndex
		&& OpenEquipmentKey == InEquipmentKey;
}

void UEquipPartsWindowWidget::SetWindowScreenPosition(FVector2D InViewportPos)
{
	FVector2D FinalPos = InViewportPos;
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	const FVector2D Desired = GetDesiredSize();

	if (ViewportSize.X > 0.f && ViewportSize.Y > 0.f)
	{
		FinalPos.X = FMath::Clamp(FinalPos.X, 0.f, FMath::Max(0.f, ViewportSize.X - Desired.X));
		FinalPos.Y = FMath::Clamp(FinalPos.Y, 0.f, FMath::Max(0.f, ViewportSize.Y - Desired.Y));
	}

	SetPositionInViewport(FinalPos, false);
}

void UEquipPartsWindowWidget::HandleCloseClicked()
{
	CloseWindow();
}

void UEquipPartsWindowWidget::HandleEquipmentChanged(int32 ChangedPartyIndex)
{
	if (ChangedPartyIndex != OpenPartyIndex) return;
	if (GetVisibility() != ESlateVisibility::Visible) return;
	RefreshWindow();
}

void UEquipPartsWindowWidget::HandleSlotEquipRequested(FName SlotSocketName, UWeaponPartDataAsset* PartToEquip)
{
	if (!EquipmentSubsystem || OpenPartyIndex == INDEX_NONE || !PartToEquip) return;

	if (EquipmentSubsystem->EquipWeaponPart(OpenPartyIndex, SlotSocketName, PartToEquip))
	{
		RefreshWindow();
	}
}

void UEquipPartsWindowWidget::HandleSlotUnequipRequested(FName SlotSocketName)
{
	if (!EquipmentSubsystem || OpenPartyIndex == INDEX_NONE) return;

	if (EquipmentSubsystem->UnequipWeaponPart(OpenPartyIndex, SlotSocketName))
	{
		RefreshWindow();
	}
}

void UEquipPartsWindowWidget::RefreshWindow()
{
	if (!Grid_Slots || !ResolvedWeapon || !SlotWidgetClass)
	{
		return;
	}

	Grid_Slots->ClearChildren();

	if (Text_Title)
	{
		Text_Title->SetText(ResolvedWeapon->DisplayName.IsEmpty()
			? FText::FromName(OpenEquipmentKey)
			: ResolvedWeapon->DisplayName);
	}

	for (int32 SlotIndex = 0; SlotIndex < ResolvedWeapon->ModSlots.Num(); ++SlotIndex)
	{
		const FWeaponModSlotDef& Def = ResolvedWeapon->ModSlots[SlotIndex];
		if (Def.SocketName == NAME_None) continue;

		UEquipPartsSlotWidget* SlotWidget = CreateWidget<UEquipPartsSlotWidget>(GetOwningPlayer(), SlotWidgetClass);
		if (!SlotWidget) continue;

		SlotWidget->InitSlot(Def.SocketName, Def.DisplayName.IsEmpty() ? FText::FromName(Def.SlotId) : Def.DisplayName);

		if (EquipmentSubsystem)
		{
			SlotWidget->SetEquipped(EquipmentSubsystem->GetEquippedPart(OpenPartyIndex, Def.SocketName));
		}

		TArray<UWeaponPartDataAsset*> CompatibleParts;
		GatherCompatibleParts(ResolvedWeapon, Def.SocketName, CompatibleParts);
		SlotWidget->SetCompatibleParts(CompatibleParts);

		SlotWidget->OnEquipRequested.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleSlotEquipRequested);
		SlotWidget->OnEquipRequested.AddDynamic(this, &UEquipPartsWindowWidget::HandleSlotEquipRequested);
		SlotWidget->OnUnequipRequested.RemoveDynamic(this, &UEquipPartsWindowWidget::HandleSlotUnequipRequested);
		SlotWidget->OnUnequipRequested.AddDynamic(this, &UEquipPartsWindowWidget::HandleSlotUnequipRequested);

		UUniformGridSlot* GridSlot = Grid_Slots->AddChildToUniformGrid(SlotWidget, SlotIndex / 2, SlotIndex % 2);
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

bool UEquipPartsWindowWidget::ResolveEquipmentButtonDef(FPlayerInfoEquipmentButtonDef& OutDef) const
{
	UUnitDataAsset* UnitDA = GetPartyUnitData(OpenPartyIndex);
	if (!UnitDA) return false;

	for (const FPlayerInfoEquipmentButtonDef& Def : UnitDA->PlayerInfoEquipButtons)
	{
		if (Def.EquipmentKey == OpenEquipmentKey)
		{
			OutDef = Def;
			return true;
		}
	}

	return false;
}

UUnitDataAsset* UEquipPartsWindowWidget::GetPartyUnitData(int32 PartyIndex) const
{
	if (!TransferSubsystem) return nullptr;
	if (!TransferSubsystem->BattleInfo.AlliesToSpawn.IsValidIndex(PartyIndex)) return nullptr;

	const TSoftObjectPtr<UUnitDataAsset> Soft = TransferSubsystem->BattleInfo.AlliesToSpawn[PartyIndex].UnitDataAsset;
	return Soft.IsNull() ? nullptr : Soft.LoadSynchronous();
}

UWeaponDataAsset* UEquipPartsWindowWidget::ResolveTargetWeapon(const FPlayerInfoEquipmentButtonDef& Def) const
{
	if (Def.WeaponOverride)
	{
		return Def.WeaponOverride;
	}

	if (EquipmentSubsystem)
	{
		if (UWeaponDataAsset* Equipped = EquipmentSubsystem->GetEquippedWeapon(OpenPartyIndex))
		{
			return Equipped;
		}
	}

	if (UUnitDataAsset* UnitDA = GetPartyUnitData(OpenPartyIndex))
	{
		return UnitDA->DefaultWeapon;
	}

	return nullptr;
}

void UEquipPartsWindowWidget::GatherCompatibleParts(UWeaponDataAsset* Weapon, FName SlotSocketName, TArray<UWeaponPartDataAsset*>& OutParts) const
{
	OutParts.Reset();
	if (!Weapon || !InventorySubsystem || !EquipmentSubsystem) return;

	for (const FInventoryStack& Stack : InventorySubsystem->GetAllStacks())
	{
		if (Stack.Quantity <= 0 || !Stack.Item) continue;

		UWeaponPartDataAsset* Part = Cast<UWeaponPartDataAsset>(Stack.Item);
		if (!Part) continue;

		if (EquipmentSubsystem->IsPartCompatibleWithWeapon(Weapon, SlotSocketName, Part))
		{
			OutParts.AddUnique(Part);
		}
	}

	OutParts.Sort([](const UWeaponPartDataAsset& A, const UWeaponPartDataAsset& B)
	{
		return A.DisplayName.ToString() < B.DisplayName.ToString();
	});
}
