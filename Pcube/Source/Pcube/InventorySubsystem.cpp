// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"

int32 UInventorySubsystem::AddItem(UItemDataAsset* Item, int32 Amount)
{
	if (!Item || Amount <= 0) return 0;
	
	bool bChanged = false;
	int32 Remaining = Amount;
	int32 AddedTotal = 0;
	
	auto AddNewStack = [&](int32 Qty) -> bool
	{
		if (Stacks.Num() >= MaxSlots) return false;
		
		FInventoryStack NewStack;
		NewStack.Item = Item;
		NewStack.Quantity = Qty;
		Stacks.Add(NewStack);
		AddedTotal += Qty;
		return true;
	};
	
	if (Item->bStackable)
	{
		for (FInventoryStack& S : Stacks)
		{
			if (S.Item == Item && S.Quantity < Item->MaxStack)
			{
				const int32 Space = Item->MaxStack - S.Quantity;
				const int32 ToAdd = FMath::Min(Space, Remaining);
				if (ToAdd > 0)
				{
					S.Quantity += ToAdd;
					Remaining -= ToAdd;
					AddedTotal += ToAdd;
					bChanged = true;
					if (Remaining <= 0) break;
				}
			}
		}
		
		while (Remaining > 0)
		{
			const int32 ToAdd = FMath::Min(Item->MaxStack, Remaining);
			if (!AddNewStack(ToAdd)) break;
			Remaining -= ToAdd;
			bChanged = true;
		}
	}
	else
	{
		while (Remaining > 0)
		{
			if (!AddNewStack(1)) break;
			Remaining -= 1;
			bChanged = true;
		}
	}
	
	if (bChanged)
	{
		OnInventoryChanged.Broadcast();
	}
	
	return AddedTotal;
}

bool UInventorySubsystem::RemoveItem(UItemDataAsset* Item, int32 Amount)
{
	if (!Item || Amount <= 0) return false;
	
	if (GetQuantity(Item) < Amount) return false;
	
	int32 Remaining = Amount;
	
	for (int32 i = Stacks.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		FInventoryStack& S = Stacks[i];
		if (S.Item == Item)
		{
			const int32 Take = FMath::Min(S.Quantity, Remaining);
			S.Quantity -= Take;
			Remaining -= Take;

			if (S.Quantity <= 0)
			{
				Stacks.RemoveAt(i);
			}
		}
	}
	
	OnInventoryChanged.Broadcast();
	return true;
}

int32 UInventorySubsystem::GetQuantity(UItemDataAsset* Item) const
{
	if (!Item) return 0;
	
	int32 Sum = 0;
	for (const FInventoryStack& S : Stacks)
	{
		if (S.Item == Item)
		{
			Sum += S.Quantity;
		}
	}
	return Sum;
}

int32 UInventorySubsystem::GetFreeSlotCount() const
{
	return FMath::Max(0, MaxSlots - Stacks.Num());
}
