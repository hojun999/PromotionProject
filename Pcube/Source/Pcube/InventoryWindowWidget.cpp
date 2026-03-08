// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWindowWidget.h"
#include "InventorySubsystem.h"
#include "InventorySlotWidget.h"
#include "WorldHUD.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

void UInventoryWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		Inv = GI->GetSubsystem<UInventorySubsystem>();
	}

	if (Inv)
	{
		Inv->OnInventoryChanged.RemoveDynamic(this, &UInventoryWindowWidget::HandleInventoryChanged);
		Inv->OnInventoryChanged.AddDynamic(this, &UInventoryWindowWidget::HandleInventoryChanged);
	}

	if (Btn_Prev)
	{
		Btn_Prev->OnClicked.RemoveDynamic(this, &UInventoryWindowWidget::HandlePrevPage);
		Btn_Prev->OnClicked.AddDynamic(this, &UInventoryWindowWidget::HandlePrevPage);
	}
	if (Btn_Next)
	{
		Btn_Next->OnClicked.RemoveDynamic(this, &UInventoryWindowWidget::HandleNextPage);
		Btn_Next->OnClicked.AddDynamic(this, &UInventoryWindowWidget::HandleNextPage);
	}
	if (Btn_Close)
	{
		Btn_Close->OnClicked.RemoveDynamic(this, &UInventoryWindowWidget::HandleCloseClicked);
		Btn_Close->OnClicked.AddDynamic(this, &UInventoryWindowWidget::HandleCloseClicked);
	}

	RebuildGrid();
}

void UInventoryWindowWidget::NativeDestruct()
{
	if (Inv)
	{
		Inv->OnInventoryChanged.RemoveDynamic(this, &UInventoryWindowWidget::HandleInventoryChanged);
	}
	Super::NativeDestruct();
}

void UInventoryWindowWidget::Open()
{
	SetVisibility(ESlateVisibility::Visible);
	RebuildGrid();
}

void UInventoryWindowWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryWindowWidget::HandleCloseClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AWorldHUD* WHUD = Cast<AWorldHUD>(PC->GetHUD()))
		{
			WHUD->HideInventory();
			return;
		}
	}
	Close();
}

void UInventoryWindowWidget::HandleInventoryChanged()
{
	if (!Inv) return;

	const int32 Total = Inv->GetAllStacks().Num();
	const int32 MaxPage = FMath::Max(0, (Total - 1) / SlotsPerPage);
	PageIndex = FMath::Clamp(PageIndex, 0, MaxPage);

	RebuildGrid();
}

void UInventoryWindowWidget::HandlePrevPage()
{
	PageIndex = FMath::Max(0, PageIndex - 1);
	RebuildGrid();
}

void UInventoryWindowWidget::HandleNextPage()
{
	if (!Inv) return;
	const int32 Total = Inv->GetAllStacks().Num();
	const int32 MaxPage = FMath::Max(0, (Total - 1) / SlotsPerPage);

	PageIndex = FMath::Min(MaxPage, PageIndex + 1);
	RebuildGrid();
}

void UInventoryWindowWidget::RebuildGrid()
{
	if (!Grid_Inventory || !InventorySlotClass)
	{
		return;
	}

	Grid_Inventory->ClearChildren();

	const int32 Start = PageIndex * SlotsPerPage;

	const TArray<FInventoryStack>* StacksPtr = nullptr;
	if (Inv)
	{
		StacksPtr = &Inv->GetAllStacks();
	}

	for (int32 i = 0; i < SlotsPerPage; ++i)
	{
		UInventorySlotWidget* InventorySlot = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), InventorySlotClass);
		if (!InventorySlot) continue;

		InventorySlot->OnInventorySlotClicked.RemoveDynamic(this, &UInventoryWindowWidget::HandleSlotClicked);
		InventorySlot->OnInventorySlotClicked.AddDynamic(this, &UInventoryWindowWidget::HandleSlotClicked);

		const int32 StackIndex = Start + i;
		if (StacksPtr && StacksPtr->IsValidIndex(StackIndex))
		{
			const FInventoryStack& S = (*StacksPtr)[StackIndex];
			InventorySlot->InitFilled(S.Item, S.Quantity, StackIndex);
		}
		else
		{
			InventorySlot->InitEmpty(StackIndex);
		}

		const int32 Row = i / Cols;
		const int32 Col = i % Cols;
		Grid_Inventory->AddChildToUniformGrid(InventorySlot, Row, Col);
	}

	UpdatePageText();
}

void UInventoryWindowWidget::UpdatePageText()
{
	if (!Text_Page || !Inv) return;

	const int32 Total = Inv->GetAllStacks().Num();
	const int32 MaxPage = FMath::Max(0, (Total - 1) / SlotsPerPage);

	Text_Page->SetText(FText::FromString(
		FString::Printf(TEXT("%d / %d"), PageIndex + 1, MaxPage + 1)));
}

void UInventoryWindowWidget::HandleSlotClicked(int32 SlotIndex, UItemDataAsset* Item)
{
	// 인벤 창에서 클릭했을 때의 동작은 아직 미정
	// SlotIndex는 이제 페이지 기준 상대 인덱스가 아니라 인벤토리 절대 인덱스다.
}
