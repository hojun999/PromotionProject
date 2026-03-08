// Fill out your copyright notice in the Description page of Project Settings.


#include "LootWindowWidget.h"

#include "LootCorpseActor.h"
#include "LootSlotWidget.h"
#include "InventorySubsystem.h"
#include "WorldHUD.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "GameFramework/PlayerController.h"

void ULootWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_TakeAll)
	{
		Btn_TakeAll->OnClicked.RemoveDynamic(this, &ULootWindowWidget::HandleTakeAll);
		Btn_TakeAll->OnClicked.AddDynamic(this, &ULootWindowWidget::HandleTakeAll);
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.RemoveDynamic(this, &ULootWindowWidget::HandleClose);
		Btn_Close->OnClicked.AddDynamic(this, &ULootWindowWidget::HandleClose);
	}
}

void ULootWindowWidget::OpenForCorpse(ALootCorpseActor* InCorpse)
{
	Corpse = InCorpse;
	RebuildGrid();
}

void ULootWindowWidget::CloseSelf()
{
	HandleClose();
}

void ULootWindowWidget::HandleClose()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AWorldHUD* WHUD = Cast<AWorldHUD>(PC->GetHUD()))
		{
			WHUD->HideLootWindow();
			return;
		}
	}
	SetVisibility(ESlateVisibility::Collapsed);
}

void ULootWindowWidget::RebuildGrid()
{
	if (!Grid_Loot || !LootSlotClass)
	{
		return;
	}

	Grid_Loot->ClearChildren();

	const int32 TotalSlots = 8;

	TArray<FLootStack> Loot;
	if (IsValid(Corpse))
	{
		Loot = Corpse->GetLootItems();
	}

	for (int32 i = 0; i < TotalSlots; ++i)
	{
		ULootSlotWidget* LootSlot = CreateWidget<ULootSlotWidget>(GetOwningPlayer(), LootSlotClass);
		if (!LootSlot) continue;

		LootSlot->OnLootSlotClicked.RemoveDynamic(this, &ULootWindowWidget::HandleSlotClicked);
		LootSlot->OnLootSlotClicked.AddDynamic(this, &ULootWindowWidget::HandleSlotClicked);

		if (Loot.IsValidIndex(i))
		{
			LootSlot->InitFilled(Loot[i], i);
		}
		else
		{
			LootSlot->InitEmpty(i);
		}

		const int32 Row = i / 4;
		const int32 Col = i % 4;
		Grid_Loot->AddChildToUniformGrid(LootSlot, Row, Col);
	}
}

void ULootWindowWidget::HandleSlotClicked(int32 SlotIndex)
{
	if (!IsValid(Corpse)) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UInventorySubsystem* Inv = GI->GetSubsystem<UInventorySubsystem>();
	if (!Inv) return;

	const TArray<FLootStack>& Loot = Corpse->GetLootItems();
	if (!Loot.IsValidIndex(SlotIndex)) return;

	const FLootStack& Stack = Loot[SlotIndex];
	if (!Stack.Item || Stack.Count <= 0) return;

	const int32 Added = Inv->AddItem(Stack.Item, Stack.Count);
	if (Added <= 0) return;

	Corpse->RemoveLootCountAt(SlotIndex, Added);
	HandleLootChangedAfterTake();
}

void ULootWindowWidget::HandleTakeAll()
{
	if (!IsValid(Corpse)) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UInventorySubsystem* Inv = GI->GetSubsystem<UInventorySubsystem>();
	if (!Inv) return;

	for (int32 i = Corpse->GetLootItems().Num() - 1; i >= 0; --i)
	{
		const TArray<FLootStack>& Loot = Corpse->GetLootItems();
		if (!Loot.IsValidIndex(i)) continue;

		const FLootStack& Stack = Loot[i];
		if (!Stack.Item || Stack.Count <= 0) continue;

		const int32 Added = Inv->AddItem(Stack.Item, Stack.Count);
		if (Added <= 0) continue;

		Corpse->RemoveLootCountAt(i, Added);
	}

	HandleLootChangedAfterTake();
}

void ULootWindowWidget::HandleLootChangedAfterTake()
{
	if (IsValid(Corpse) && Corpse->IsEmpty())
	{
		ALootCorpseActor* ToDestroy = Corpse;
		Corpse = nullptr;

		HandleClose();
		ToDestroy->Destroy();
		return;
	}

	RebuildGrid();
}
