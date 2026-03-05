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
			WHUD->HideLootWindow(); // // HUD가 InputMode 복구까지 담당
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

	// 항상 8칸 생성(2x4)
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

	FLootStack Taken;
	if (!Corpse->TakeLootAt(SlotIndex, Taken))
	{
		return;
	}

	// 인벤으로 이동
	Inv->AddItem(Taken.Item, Taken.Count);

	HandleLootChangedAfterTake();
}

void ULootWindowWidget::HandleTakeAll()
{
	if (!IsValid(Corpse)) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UInventorySubsystem* Inv = GI->GetSubsystem<UInventorySubsystem>();
	if (!Inv) return;

	TArray<FLootStack> TakenAll;
	Corpse->TakeAllLoot(TakenAll);

	for (const FLootStack& S : TakenAll)
	{
		if (S.Item && S.Count > 0)
		{
			Inv->AddItem(S.Item, S.Count);
		}
	}

	HandleLootChangedAfterTake();
}

void ULootWindowWidget::HandleLootChangedAfterTake()
{
	// 비었으면 시체 제거 + 창 닫기
	if (IsValid(Corpse) && Corpse->IsEmpty())
	{
		ALootCorpseActor* ToDestroy = Corpse;
		Corpse = nullptr;

		// HUD 닫기
		HandleClose();

		// 시체 제거(Transfer에는 이미 MarkLooted가 저장됨)
		ToDestroy->Destroy();
		return;
	}

	RebuildGrid();
}