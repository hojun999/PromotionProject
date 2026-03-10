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
	PartyIndex = INDEX_NONE;
}

void UEquipPartsWindowWidget::OpenForWeapon(int32 InPartyIndex, FName InEquipmentButtonKey, UWeaponDataAsset* InWeapon)
{
	ResetActiveEquipment();
	PartyIndex = InPartyIndex;
	EquipmentButtonKey = InEquipmentButtonKey;
	CurrentEquipmentKind = EPlayerInfoEquipmentKind::Weapon;
	ActiveWeapon = InWeapon;
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
	SetVisibility(ESlateVisibility::Visible);
	Refresh();
}

void UEquipPartsWindowWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
	if (Canvas_Parts)
	{
		Canvas_Parts->ClearChildren();
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


void UEquipPartsWindowWidget::Refresh()
{
	if (GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}

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
	
	RebuildCandidateCanvas();
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


void UEquipPartsWindowWidget::HandlePartCandidateClicked(UWeaponPartDataAsset* Part)
{
	if (!EquipmentSubsystem || !Part || PartyIndex == INDEX_NONE)
	{
		return;
	}

	// 파츠의 CompatibleEquipmentKeys에서 이 무기/방어구에 맞는 SlotId를 찾아 장착
	const TArray<FWeaponModSlotDef>* Slots = GetActiveSlots();
	if (!Slots) return;

	for (const FWeaponModSlotDef& Def : *Slots)
	{
		// 이 슬롯의 CandidateParts 중 클릭된 파츠가 있는지 확인 
		for (const FWeaponPartSlotEntry& Entry : Def.CandidateParts) 
		{
			if (Entry.Part.Get() == Part)
			{
				EquipmentSubsystem->EquipWeaponPart(PartyIndex, Def.SlotId, Part); 
				return;
			}
		}
	}
}

void UEquipPartsWindowWidget::ClearCandidateCanvas()
{
	if (Canvas_Parts)
	{
		Canvas_Parts->ClearChildren();
	}
}

void UEquipPartsWindowWidget::RebuildCandidateCanvas()
{
	if (!Canvas_Parts)
	{
		return;
	}

	ClearCandidateCanvas();

	if (!InventorySubsystem || !EquipmentSubsystem || !PartCandidateWidgetClass || PartyIndex == INDEX_NONE)
	{
		return;
	}

	const TArray<FWeaponModSlotDef>* Slots = GetActiveSlots();
	if (!Slots)
	{
		return;
	}

	// 모든 슬롯의 CandidateParts를 한꺼번에 표시
	for (const FWeaponModSlotDef& Def : *Slots)
	{
		for (const FWeaponPartSlotEntry& Entry : Def.CandidateParts)
		{
			UWeaponPartDataAsset* Part = Entry.Part.LoadSynchronous();
			if (!Part) continue;

			// GetQuantity로 직접 조회 (포인터 불일치 방지)
			const int32 InvQty = InventorySubsystem->GetQuantity(Part);

			UE_LOG(LogTemp, Warning, TEXT("[EquipParts] Part=%s Ptr=%p InvQty=%d"), *GetNameSafe(Part), Part, InvQty);
			
			UEquipPartsSlotWidget* CandidateWidget = CreateWidget<UEquipPartsSlotWidget>(GetOwningPlayer(), PartCandidateWidgetClass);
			if (!CandidateWidget) continue;

			CandidateWidget->Init(Part, InvQty);
			CandidateWidget->OnClicked.RemoveDynamic(this, &UEquipPartsWindowWidget::HandlePartCandidateClicked);
			CandidateWidget->OnClicked.AddDynamic(this, &UEquipPartsWindowWidget::HandlePartCandidateClicked);

			if (UCanvasPanelSlot* CanvasSlot = Canvas_Parts->AddChildToCanvas(CandidateWidget))
			{
				CanvasSlot->SetAutoSize(false);
				CanvasSlot->SetAnchors(FAnchors(Entry.UIAnchor.X, Entry.UIAnchor.Y, Entry.UIAnchor.X, Entry.UIAnchor.Y));
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasSlot->SetPosition(Entry.UIPixelOffset);
				const FVector2D FinalSize = (Entry.UISize.X > 0.f && Entry.UISize.Y > 0.f) ? Entry.UISize : DefaultSlotWidgetSize;
				CanvasSlot->SetSize(FinalSize);
			}
		}
	}
}
