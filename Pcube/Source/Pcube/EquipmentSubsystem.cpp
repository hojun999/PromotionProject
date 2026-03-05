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
			return &Def;
	}
	return nullptr;
}

bool UEquipmentSubsystem::EquipWeapon(int32 PartyIndex, UWeaponDataAsset* NewWeapon)
{
	if (!IsValidParty(PartyIndex) || !NewWeapon) return false;

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	// 기존 장착 파츠 전부 반환
	for (auto& KVP : PartyWeapons[PartyIndex].EquippedParts)
	{
		if (KVP.Value)
		{
			Inv->AddItem(KVP.Value, 1);
		}
	}
	PartyWeapons[PartyIndex].EquippedParts.Empty();

	PartyWeapons[PartyIndex].Weapon = NewWeapon;
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

	// 호환 검사: SlotType
	if (NewPart->SlotType != SlotDef->SlotType) return false;

	// 소켓 제한(명시된 파츠만)
	if (NewPart->AttachmentSocketName != NAME_None && NewPart->AttachmentSocketName != SlotDef->SocketName)
	{
		return false;
	}

	// 인벤에 없으면 장착 불가
	if (Inv->GetQuantity(NewPart) <= 0) return false;

	// 기존 장착품
	UWeaponPartDataAsset* Old = GetEquippedPart(PartyIndex, SlotDef->SocketName);
	if (Old == NewPart) return true;

	// 새 파츠 소비
	if (!Inv->RemoveItem(NewPart, 1)) return false;

	// 기존 파츠 반환
	if (Old) Inv->AddItem(Old, 1);

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

	Inv->AddItem(Old, 1);
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