#include "EquipmentSubsystem.h"

#include "ArmorDataAsset.h"
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

	// UnitData가 바뀌었으면 장비 상태 리셋 후 재초기화 
	if (State.SourceUnitData != UnitData) 
	{ 
		State.Weapon = nullptr; 
		State.Armor = nullptr; 
		State.EquippedParts.Empty(); 
		State.SourceUnitData = UnitData; 
		UE_LOG(LogTemp, Warning, TEXT("[EquipSub] PartyIndex=%d UnitData 변경 감지 - 장비 리셋: %s"), PartyIndex, *GetNameSafe(UnitData)); 
	} 

	if (!State.Weapon && UnitData->DefaultWeapon)
	{
		State.Weapon = UnitData->DefaultWeapon;
	}

	if (!State.Armor && UnitData->DefaultArmor)
	{
		State.Armor = UnitData->DefaultArmor;
	}
}


const FWeaponModSlotDef* UEquipmentSubsystem::FindWeaponSlotDef(const UWeaponDataAsset* Weapon, FName PartSlotKeyOrSocketName) const
{
	if (!Weapon || PartSlotKeyOrSocketName == NAME_None) return nullptr;

	for (const FWeaponModSlotDef& Def : Weapon->ModSlots)
	{
		if (Def.SlotId == PartSlotKeyOrSocketName || Def.SocketName == PartSlotKeyOrSocketName)
		{
			return &Def;
		}
	}
	return nullptr;
}

FName UEquipmentSubsystem::ResolveStoredPartKey(int32 PartyIndex, FName PartSlotKeyOrSocketName) const
{
	if (!IsValidParty(PartyIndex) || PartSlotKeyOrSocketName == NAME_None) return NAME_None;

	const FUnitEquipmentState& State = PartyEquipments[PartyIndex];

	if (State.EquippedParts.Contains(PartSlotKeyOrSocketName))
	{
		return PartSlotKeyOrSocketName;
	}

	if (const FWeaponModSlotDef* WeaponDef = FindWeaponSlotDef(State.Weapon, PartSlotKeyOrSocketName))
	{
		return WeaponDef->SlotId;
	}

	if (const FWeaponModSlotDef* ArmorDef = FindArmorSlotDef(State.Armor, PartSlotKeyOrSocketName))
	{
		return ArmorDef->SlotId;
	}

	return PartSlotKeyOrSocketName;
}

const FWeaponModSlotDef* UEquipmentSubsystem::FindArmorSlotDef(const UArmorDataAsset* Armor, FName PartSlotKeyOrSocketName) const
{
	if (!Armor || PartSlotKeyOrSocketName == NAME_None) return nullptr;

	for (const FWeaponModSlotDef& Def : Armor->ModSlots)
	{
		if (Def.SlotId == PartSlotKeyOrSocketName || Def.SocketName == PartSlotKeyOrSocketName)
		{
			return &Def;
		}
	}
	return nullptr;
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

	// 무기 교체 시에는 기존 무기 슬롯에 장착된 파츠만 반환
	if (State.Weapon)
	{
		for (const FWeaponModSlotDef& Def : State.Weapon->ModSlots)
		{
			if (UWeaponPartDataAsset* OldPart = GetEquippedPart(PartyIndex, Def.SlotId))
			{
				Inv->AddItem(OldPart, 1);
				State.EquippedParts.Remove(Def.SlotId);
			}
		}
	}
	
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

UWeaponPartDataAsset* UEquipmentSubsystem::GetEquippedPart(int32 PartyIndex, FName PartSlotKeyOrSocketName) const
{
	if (!IsValidParty(PartyIndex) || PartSlotKeyOrSocketName == NAME_None) return nullptr;

	const FName StoredKey = ResolveStoredPartKey(PartyIndex, PartSlotKeyOrSocketName);
	if (StoredKey == NAME_None) return nullptr;

	if (const TObjectPtr<UWeaponPartDataAsset>* Found = PartyEquipments[PartyIndex].EquippedParts.Find(StoredKey))
	{
		return Found->Get();
	}
	return nullptr;
}

bool UEquipmentSubsystem::CanEquipWeaponPart(int32 PartyIndex, FName PartSlotKeyOrSocketName, UWeaponPartDataAsset* NewPart) const
{
	if (!IsValidParty(PartyIndex) || !NewPart || PartSlotKeyOrSocketName == NAME_None)
	{
		return false;
	}

	const FName StoredKey = ResolveStoredPartKey(PartyIndex, PartSlotKeyOrSocketName);
	if (StoredKey == NAME_None)
	{
		return false;
	}

	// 직접 키 매칭
	if (!NewPart->CompatibleEquipmentKeys.IsEmpty())
	{
		if (!NewPart->IsCompatibleWithEquipmentKey(StoredKey))
		{
			return false;
		}
	}
	else
	{
		// 레거시 호환 (SlotType / SocketName)
		const FUnitEquipmentState& State = PartyEquipments[PartyIndex];
		const FWeaponModSlotDef* SlotDef = FindWeaponSlotDef(State.Weapon, StoredKey);
		if (!SlotDef)
		{
			SlotDef = FindArmorSlotDef(State.Armor, StoredKey);
		}
		if (!SlotDef) return false;
		if (NewPart->SlotType != SlotDef->SlotType) return false;
		if (NewPart->AttachmentSocketName != NAME_None && NewPart->AttachmentSocketName != SlotDef->SocketName) return false;
	}

	const UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	const int32 InvQuantity = Inv->GetQuantity(NewPart);
	if (InvQuantity <= 0) return false;

	// 동일 파츠가 모든 슬롯에 장착된 수량 카운트 // 추가됨
	int32 AlreadyEquippedCount = 0; // 추가됨
	for (const FUnitEquipmentState& EqState : PartyEquipments) // 추가됨
	{ // 추가됨
		for (const TPair<FName, TObjectPtr<UWeaponPartDataAsset>>& Pair : EqState.EquippedParts) // 추가됨
		{ // 추가됨
			if (Pair.Value == NewPart) ++AlreadyEquippedCount; // 추가됨
		} // 추가됨
	} // 추가됨

	// 현재 슬롯에 이미 같은 파츠가 장착됨 = 재장착 불필요 → false // 수정됨
	if (const TObjectPtr<UWeaponPartDataAsset>* CurPart = PartyEquipments[PartyIndex].EquippedParts.Find(StoredKey)) // 수정됨
	{ // 수정됨
		if (*CurPart == NewPart) return false; // 수정됨 - 같은 슬롯 같은 파츠 → 버튼 비활성화
		--AlreadyEquippedCount; // 수정됨 - 다른 파츠면 교체 대상이므로 차감
	} // 수정됨

	return (InvQuantity - AlreadyEquippedCount) > 0; // 수정됨
}





bool UEquipmentSubsystem::EquipWeaponPart(int32 PartyIndex, FName PartSlotKeyOrSocketName, UWeaponPartDataAsset* NewPart)
{
	if (!CanEquipWeaponPart(PartyIndex, PartSlotKeyOrSocketName, NewPart))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipPart] CanEquip FAILED PartyIndex=%d SlotKey=%s Part=%s"),
			PartyIndex, *PartSlotKeyOrSocketName.ToString(), *GetNameSafe(NewPart));
		return false;
	}

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	const FName StoredKey = ResolveStoredPartKey(PartyIndex, PartSlotKeyOrSocketName);
	if (StoredKey == NAME_None) return false;
	
	FUnitEquipmentState& State = PartyEquipments[PartyIndex];
	UWeaponPartDataAsset* OldPart = GetEquippedPart(PartyIndex, StoredKey);
	if (OldPart == NewPart)
	{
		return false;
	}

	if (!Inv->RemoveItem(NewPart, 1))
	{
		return false;
	}

	if (OldPart)
	{
		Inv->AddItem(OldPart, 1);
	}

	State.EquippedParts.FindOrAdd(StoredKey) = NewPart;
	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}



bool UEquipmentSubsystem::UnequipWeaponPart(int32 PartyIndex, FName PartSlotKeyOrSocketName)
{
	if (!IsValidParty(PartyIndex) || PartSlotKeyOrSocketName == NAME_None) return false;

	UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	const FName StoredKey = ResolveStoredPartKey(PartyIndex, PartSlotKeyOrSocketName);
	if (StoredKey == NAME_None) return false;
	
	UWeaponPartDataAsset* OldPart = GetEquippedPart(PartyIndex, StoredKey);
	if (!OldPart) return false;

	Inv->AddItem(OldPart, 1);
	PartyEquipments[PartyIndex].EquippedParts.Remove(StoredKey);
	
	OnEquipmentChanged.Broadcast(PartyIndex);
	return true;
}

FWeaponPartEffect UEquipmentSubsystem::GetTotalEquippedPartEffect(int32 PartyIndex) const
{
	FWeaponPartEffect Out;
	Out.DamageMulMul = 1.f;
	if (!IsValidParty(PartyIndex)) return Out;

	for (const TPair<FName, TObjectPtr<UWeaponPartDataAsset>>& Pair : PartyEquipments[PartyIndex].EquippedParts)
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
		Out.MaxHP += State.Weapon->BaseStatBonus.MaxHP;
		Out.Speed += State.Weapon->BaseStatBonus.Speed;
		Out.AttackPower += State.Weapon->BaseStatBonus.AttackPower;
	}
	if (State.Armor)
	{
		Out.MaxHP += State.Armor->BaseStatBonus.MaxHP;
		Out.Speed += State.Armor->BaseStatBonus.Speed;
		Out.AttackPower += State.Armor->BaseStatBonus.AttackPower;
	}

	return Out;
}

FUnitBaseStats UEquipmentSubsystem::ResolveFinalStats(int32 PartyIndex, const FUnitBaseStats& BaseStats) const
{
	FUnitBaseStats Out = BaseStats;
	const FEquipmentStatBonus Bonus = GetTotalEquipmentBaseStatBonus(PartyIndex);
	Out.MaxHP = FMath::Max(1.f, Out.MaxHP + Bonus.MaxHP);
	Out.Speed = FMath::Max(1.f, Out.Speed + Bonus.Speed);
	Out.AttackPower = FMath::Max(1.f, Out.AttackPower + Bonus.AttackPower);
	return Out;
}