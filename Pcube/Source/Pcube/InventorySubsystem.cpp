// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"

void UInventorySubsystem::AddItem(UItemDataAsset* Item, int32 Amount)
{
	if (!Item || Amount <= 0) return;
	
	bool bChanged = false;
	int32 Remaining = Amount;
	
	auto AddNewStack = [&](int32 Qty) -> bool
	{
		if (Stacks.Num() >= MaxSlots) return false;
		
		FInventoryStack NewStack;
		NewStack.Item = Item;
		NewStack.Quantity = Qty;
		Stacks.Add(NewStack);
		return true;
	};
	
	if (Item->bStackable)
	{
		// 기존 스택 채우기
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
					bChanged = true;
					if (Remaining <= 0) break;
				}
			}
		}
		
		// 새 스택 생성
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
		// 비스택 아이템: 1개당 1슬롯
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
	
	// Remaining > 0 이면 슬롯 부족으로 일부 실패
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
