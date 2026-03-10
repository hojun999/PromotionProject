#include "EquipmentSubsystem.h"
#include "InventorySubsystem.h"
#include "UnitDataAsset.h"

namespace
{
	static void AccumulateStatBonus(FEquipmentStatBonus& OutBonus, const FEquipmentStatBonus& InBonus)
	{
		OutBonus.MaxHP += InBonus.MaxHP;
		OutBonus.Speed += InBonus.Speed;
		OutBonus.AttackPower += InBonus.AttackPower;
	}
}

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
	return PartyEquipments.IsValidIndex(PartyIndex);
}

void UEquipmentSubsystem::EnsurePartySize(int32 NumPartyMembers)
{
	if (NumPartyMembers <= 0) return;
	if (PartyEquipments.Num() < NumPartyMembers)
	{
		PartyEquipments.SetNum(NumPartyMembers);
	}
}

void UEquipmentSubsystem::InitializeUnitLoadoutIfMissing(int32 PartyIndex, UUnitDataAsset* UnitData)
{
	if (!UnitData || PartyIndex < 0) return;

	EnsurePartySize(PartyIndex + 1);
	if (!IsValidParty(PartyIndex)) return;

	FUnitEquipmentState& State = PartyEquipments[PartyIndex];

	if (!State.Weapon && UnitData->DefaultWeapon)
	{
		State.Weapon = UnitData->DefaultWeapon;
	}

	if (!State.Armor && UnitData->DefaultArmor)
	{
		State.Armor = UnitData->DefaultArmor;
	}
}

bool UEquipmentSubsystem::EquipWeapon(int32 PartyIndex, UWeaponDataAsset* NewWeapon)
{
	if (!IsValidParty(PartyIndex) || !NewWeapon) return false;

	FUnitEquipmentState& State = PartyEquipments[PartyIndex];
	if (State.Weapon == NewWeapon)
	{
		return true;
	}

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	for (auto& Pair : State.EquippedParts)
	{
		if (Pair.Value)
		{
			Inv->AddItem(Pair.Value, 1);
		}
	}
	State.EquippedParts.Empty();
	State.Weapon = NewWeapon;

	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}

UWeaponDataAsset* UEquipmentSubsystem::GetEquippedWeapon(int32 PartyIndex) const
{
	return IsValidParty(PartyIndex) ? PartyEquipments[PartyIndex].Weapon : nullptr;
}

UArmorDataAsset* UEquipmentSubsystem::GetEquippedArmor(int32 PartyIndex) const
{
	return IsValidParty(PartyIndex) ? PartyEquipments[PartyIndex].Armor : nullptr;
}

UWeaponPartDataAsset* UEquipmentSubsystem::GetEquippedPart(int32 PartyIndex, FName PartSlotKey) const
{
	if (!IsValidParty(PartyIndex) || PartSlotKey == NAME_None) return nullptr;
	if (const TObjectPtr<UWeaponPartDataAsset>* Found = PartyEquipments[PartyIndex].EquippedParts.Find(PartSlotKey))
	{
		return Found->Get();
	}
	return nullptr;
}

bool UEquipmentSubsystem::CanEquipWeaponPart(int32 PartyIndex, FName PartSlotKey, UWeaponPartDataAsset* NewPart) const
{
	if (!IsValidParty(PartyIndex) || PartSlotKey == NAME_None || !NewPart)
	{
		return false;
	}

	if (!NewPart->IsCompatibleWithEquipmentKey(PartSlotKey))
	{
		return false;
	}

	const UInventorySubsystem* Inv = GetInv();
	return Inv && Inv->GetQuantity(NewPart) > 0;
}

bool UEquipmentSubsystem::EquipWeaponPart(int32 PartyIndex, FName PartSlotKey, UWeaponPartDataAsset* NewPart)
{
	if (!CanEquipWeaponPart(PartyIndex, PartSlotKey, NewPart))
	{
		return false;
	}

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	FUnitEquipmentState& State = PartyEquipments[PartyIndex];
	UWeaponPartDataAsset* OldPart = GetEquippedPart(PartyIndex, PartSlotKey);
	if (OldPart == NewPart)
	{
		return true;
	}

	if (!Inv->RemoveItem(NewPart, 1))
	{
		return false;
	}

	if (OldPart)
	{
		Inv->AddItem(OldPart, 1);
	}

	State.EquippedParts.FindOrAdd(PartSlotKey) = NewPart;
	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}


bool UEquipmentSubsystem::UnequipWeaponPart(int32 PartyIndex, FName PartSlotKey)
{
	if (!IsValidParty(PartyIndex) || PartSlotKey == NAME_None) return false;

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	UWeaponPartDataAsset* OldPart = GetEquippedPart(PartyIndex, PartSlotKey);
	if (!OldPart) return false;

	Inv->AddItem(OldPart, 1);
	PartyEquipments[PartyIndex].EquippedParts.Remove(PartSlotKey);
	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}

FWeaponPartEffect UEquipmentSubsystem::GetTotalEquippedPartEffect(int32 PartyIndex) const
{
	FWeaponPartEffect Out;
	Out.DamageMulMul = 1.f;

	if (!IsValidParty(PartyIndex)) return Out;

	for (const auto& Pair : PartyEquipments[PartyIndex].EquippedParts)
	{
		const UWeaponPartDataAsset* Part = Pair.Value;
		if (!Part) continue;

		Out.BonusProjectileCount += Part->Effect.BonusProjectileCount;
		Out.BonusHitCount += Part->Effect.BonusHitCount;
		Out.DamageMulAdd += Part->Effect.DamageMulAdd;
		Out.DamageMulMul *= Part->Effect.DamageMulMul;
	}

	return Out;
}

FEquipmentStatBonus UEquipmentSubsystem::GetTotalEquipmentBaseStatBonus(int32 PartyIndex) const
{
	FEquipmentStatBonus Out;

	if (!IsValidParty(PartyIndex)) return Out;

	const FUnitEquipmentState& State = PartyEquipments[PartyIndex];
	if (State.Weapon)
	{
		AccumulateStatBonus(Out, State.Weapon->BaseStatBonus);
	}
	if (State.Armor)
	{
		AccumulateStatBonus(Out, State.Armor->BaseStatBonus);
	}

	return Out;
}

FUnitBaseStats UEquipmentSubsystem::ResolveFinalStats(int32 PartyIndex, const FUnitBaseStats& BaseStats) const
{
	const FEquipmentStatBonus Bonus = GetTotalEquipmentBaseStatBonus(PartyIndex);

	FUnitBaseStats Out = BaseStats;
	Out.MaxHP = FMath::Max(1.f, BaseStats.MaxHP + Bonus.MaxHP);
	Out.Speed = FMath::Max(1.f, BaseStats.Speed + Bonus.Speed);
	Out.AttackPower = FMath::Max(1.f, BaseStats.AttackPower + Bonus.AttackPower);
	return Out;
}


// FWeaponPartEffect UEquipmentSubsystem::GetTotalWeaponPartEffect(int32 PartyIndex) const
// {
// 	FWeaponPartEffect Out;
// 	Out.DamageMulMul = 1.f;
//
// 	if (!IsValidParty(PartyIndex)) return Out;
//
// 	for (const auto& KVP : PartyWeapons[PartyIndex].EquippedParts)
// 	{
// 		const UWeaponPartDataAsset* P = KVP.Value;
// 		if (!P) continue;
//
// 		Out.BonusProjectileCount += P->Effect.BonusProjectileCount;
// 		Out.BonusHitCount += P->Effect.BonusHitCount;
// 		Out.DamageMulAdd += P->Effect.DamageMulAdd;
// 		Out.DamageMulMul *= P->Effect.DamageMulMul;
// 	}
// 	return Out;
// }
