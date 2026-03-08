#include "EquipmentSubsystem.h"
#include "InventorySubsystem.h"

UInventorySubsystem* UEquipmentSubsystem::GetInv() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UInventorySubsystem>();
	}
	return nullptr;
}

bool UEquipmentSubsystem::IsValidParty(int32 PartyIndex) const
{
	return PartyWeapons.IsValidIndex(PartyIndex);
}

void UEquipmentSubsystem::EnsurePartySize(int32 NumPartyMembers)
{
	if (NumPartyMembers <= 0) return;
	if (PartyWeapons.Num() < NumPartyMembers)
	{
		PartyWeapons.SetNum(NumPartyMembers);
	}
}

const FWeaponModSlotDef* UEquipmentSubsystem::FindSlotDef(UWeaponDataAsset* Weapon, FName SlotSocketName) const
{
	if (!Weapon || SlotSocketName == NAME_None) return nullptr;
	for (const FWeaponModSlotDef& Def : Weapon->ModSlots)
	{
		if (Def.SocketName == SlotSocketName)
		{
			return &Def;
		}
	}
	return nullptr;
}

int32 UEquipmentSubsystem::CountEquippedParts(int32 PartyIndex) const
{
	if (!IsValidParty(PartyIndex)) return 0;

	int32 Count = 0;
	for (const auto& Pair : PartyWeapons[PartyIndex].EquippedParts)
	{
		if (Pair.Value)
		{
			++Count;
		}
	}
	return Count;
}

bool UEquipmentSubsystem::EquipWeapon(int32 PartyIndex, UWeaponDataAsset* NewWeapon)
{
	if (!IsValidParty(PartyIndex) || !NewWeapon) return false;

	FUnitWeaponState& State = PartyWeapons[PartyIndex];
	if (State.Weapon == NewWeapon)
	{
		return true;
	}

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	const int32 PartsToReturn = CountEquippedParts(PartyIndex);
	if (PartsToReturn > 0 && Inv->GetFreeSlotCount() < PartsToReturn)
	{
		return false;
	}

	for (auto& KVP : State.EquippedParts)
	{
		if (KVP.Value)
		{
			if (Inv->AddItem(KVP.Value, 1) != 1)
			{
				return false;
			}
		}
	}
	State.EquippedParts.Empty();

	State.Weapon = NewWeapon;
	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}

UWeaponDataAsset* UEquipmentSubsystem::GetEquippedWeapon(int32 PartyIndex) const
{
	return IsValidParty(PartyIndex) ? PartyWeapons[PartyIndex].Weapon : nullptr;
}

UWeaponPartDataAsset* UEquipmentSubsystem::GetEquippedPart(int32 PartyIndex, FName SlotSocketName) const
{
	if (!IsValidParty(PartyIndex)) return nullptr;
	if (const TObjectPtr<UWeaponPartDataAsset>* Found = PartyWeapons[PartyIndex].EquippedParts.Find(SlotSocketName))
	{
		return Found->Get();
	}
	return nullptr;
}

bool UEquipmentSubsystem::EquipWeaponPart(int32 PartyIndex, FName SlotSocketName, UWeaponPartDataAsset* NewPart)
{
	if (!IsValidParty(PartyIndex) || !NewPart || SlotSocketName == NAME_None) return false;

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	UWeaponDataAsset* Weapon = PartyWeapons[PartyIndex].Weapon;
	const FWeaponModSlotDef* SlotDef = FindSlotDef(Weapon, SlotSocketName);
	if (!SlotDef) return false;

	if (NewPart->SlotType != SlotDef->SlotType) return false;

	if (NewPart->AttachmentSocketName != NAME_None && NewPart->AttachmentSocketName != SlotDef->SocketName)
	{
		return false;
	}

	UWeaponPartDataAsset* Old = GetEquippedPart(PartyIndex, SlotDef->SocketName);
	if (Old == NewPart)
	{
		return true;
	}

	if (Inv->GetQuantity(NewPart) <= 0) return false;
	if (!Inv->RemoveItem(NewPart, 1)) return false;

	if (Old)
	{
		if (Inv->AddItem(Old, 1) != 1)
		{
			Inv->AddItem(NewPart, 1);
			return false;
		}
	}

	PartyWeapons[PartyIndex].EquippedParts.FindOrAdd(SlotDef->SocketName) = NewPart;
	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}

bool UEquipmentSubsystem::UnequipWeaponPart(int32 PartyIndex, FName SlotSocketName)
{
	if (!IsValidParty(PartyIndex) || SlotSocketName == NAME_None) return false;

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	UWeaponPartDataAsset* Old = GetEquippedPart(PartyIndex, SlotSocketName);
	if (!Old) return false;

	if (Inv->GetFreeSlotCount() < 1)
	{
		return false;
	}

	if (Inv->AddItem(Old, 1) != 1)
	{
		return false;
	}

	PartyWeapons[PartyIndex].EquippedParts.Remove(SlotSocketName);

	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}

FWeaponPartEffect UEquipmentSubsystem::GetTotalWeaponPartEffect(int32 PartyIndex) const
{
	FWeaponPartEffect Out;
	Out.DamageMulMul = 1.f;

	if (!IsValidParty(PartyIndex)) return Out;

	for (const auto& KVP : PartyWeapons[PartyIndex].EquippedParts)
	{
		const UWeaponPartDataAsset* P = KVP.Value;
		if (!P) continue;

		Out.BonusProjectileCount += P->Effect.BonusProjectileCount;
		Out.BonusHitCount += P->Effect.BonusHitCount;
		Out.DamageMulAdd += P->Effect.DamageMulAdd;
		Out.DamageMulMul *= P->Effect.DamageMulMul;
	}
	return Out;
}
