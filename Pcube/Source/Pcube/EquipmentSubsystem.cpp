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

	if (!State.Weapon && UnitData->DefaultWeapon)
	{
		State.Weapon = UnitData->DefaultWeapon;
	}

	// PlayerInfoEquipButtons 기준으로 모든 방어구 초기화 
	for (const FPlayerInfoEquipmentButtonDef& Btn : UnitData->PlayerInfoEquipButtons) 
	{
		if (Btn.Kind != EPlayerInfoEquipmentKind::Armor) continue; 
		if (Btn.EquipmentKey == NAME_None) continue; 
		UArmorDataAsset* DefaultArmor = Btn.ArmorOverride ? Btn.ArmorOverride : UnitData->DefaultArmor; 
		if (DefaultArmor && !State.Armors.Contains(Btn.EquipmentKey)) 
		{
			State.Armors.Add(Btn.EquipmentKey, DefaultArmor); 
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[EquipSub] InitLoadout PartyIndex=%d UnitData=%s Weapon=%s ArmorCount=%d"), 
		PartyIndex, *GetNameSafe(UnitData), *GetNameSafe(State.Weapon), State.Armors.Num()); 
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

	for (const TPair<FName, TObjectPtr<UArmorDataAsset>>& ArmorPair : State.Armors)
	{
		if (const FWeaponModSlotDef* ArmorDef = FindArmorSlotDef(ArmorPair.Value, PartSlotKeyOrSocketName))
		{
			return ArmorDef->SlotId;
		}
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

UArmorDataAsset* UEquipmentSubsystem::GetEquippedArmor(int32 PartyIndex, FName ArmorID) const 
{
	if (!IsValidParty(PartyIndex) || ArmorID == NAME_None) return nullptr; 
	if (const TObjectPtr<UArmorDataAsset>* Found = PartyEquipments[PartyIndex].Armors.Find(ArmorID)) 
	{
		return Found->Get(); 
	}
	return nullptr; 
}

const TMap<FName, TObjectPtr<UArmorDataAsset>>* UEquipmentSubsystem::GetAllEquippedArmors(int32 PartyIndex) const 
{
	return IsValidParty(PartyIndex) ? &PartyEquipments[PartyIndex].Armors : nullptr; 
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
		if (!SlotDef) // Armors 맵 순회
		{
			for (const TPair<FName, TObjectPtr<UArmorDataAsset>>& ArmorPair : State.Armors) 
			{
				SlotDef = FindArmorSlotDef(ArmorPair.Value, StoredKey); 
				if (SlotDef) break; 
			}
		}
		if (!SlotDef) return false;
		if (NewPart->SlotType != SlotDef->SlotType) return false;
		if (NewPart->AttachmentSocketName != NAME_None && NewPart->AttachmentSocketName != SlotDef->SocketName) return false;
	}

	const UInventorySubsystem* Inv = GetInv();
	if (!Inv) return false;

	const int32 InvQuantity = Inv->GetQuantity(NewPart);
	if (InvQuantity <= 0) return false;

	// 동일 파티원 내 슬롯에 장착된 수량만 카운트, 전체 파티 → 같은 파티원 내
	int32 AlreadyEquippedCount = 0;
	for (const TPair<FName, TObjectPtr<UWeaponPartDataAsset>>& Pair : PartyEquipments[PartyIndex].EquippedParts)
	{
		if (Pair.Value == NewPart) ++AlreadyEquippedCount;
	}

	// 현재 슬롯에 이미 같은 파츠가 장착됨 = 재장착 불필요 → false 
	if (const TObjectPtr<UWeaponPartDataAsset>* CurPart = PartyEquipments[PartyIndex].EquippedParts.Find(StoredKey)) 
	{ 
		if (*CurPart == NewPart) return false; // 같은 슬롯 같은 파츠 → 버튼 비활성화
		--AlreadyEquippedCount; // 다른 파츠면 교체 대상이므로 차감
	} 

	UE_LOG(LogTemp, Warning, TEXT("[CanEquip] InvQty=%d AlreadyEquipped=%d StoredKey=%s"),
	InvQuantity, AlreadyEquippedCount, *StoredKey.ToString());
	return InvQuantity > 0; 
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
	Out.BonusSkillPointGain = 0; 
	if (!IsValidParty(PartyIndex)) return Out;

	for (const TPair<FName, TObjectPtr<UWeaponPartDataAsset>>& Pair : PartyEquipments[PartyIndex].EquippedParts)
	{
		const UWeaponPartDataAsset* Part = Pair.Value;
		if (!Part) continue;
		Out.BonusProjectileCount += Part->Effect.BonusProjectileCount;
		Out.BonusHitCount += Part->Effect.BonusHitCount;
		Out.BonusSkillPointGain += Part->Effect.BonusSkillPointGain;
		Out.BonusMaxHP += Part->Effect.BonusMaxHP;
		Out.BonusAttackPower += Part->Effect.BonusAttackPower;
		Out.BonusSpeed += Part->Effect.BonusSpeed;
	}
	return Out;
}


FEquipmentStatBonus UEquipmentSubsystem::GetTotalEquipmentBaseStatBonus(int32 PartyIndex) const
{
	FEquipmentStatBonus Out;
	if (!IsValidParty(PartyIndex)) return Out;

	const FUnitEquipmentState& State = PartyEquipments[PartyIndex];
	UE_LOG(LogTemp, Warning, TEXT("[EquipSub] GetStatBonus PartyIndex=%d Weapon=%s ArmorCount=%d"),
		PartyIndex, *GetNameSafe(State.Weapon), State.Armors.Num()); 
	if (State.Weapon)
	{
		Out.MaxHP += State.Weapon->BaseStatBonus.MaxHP;
		Out.Speed += State.Weapon->BaseStatBonus.Speed;
		Out.AttackPower += State.Weapon->BaseStatBonus.AttackPower;
		UE_LOG(LogTemp, Warning, TEXT("[EquipSub]  Weapon bonus: HP=%.1f SPD=%.1f ATK=%.1f"),
			State.Weapon->BaseStatBonus.MaxHP, State.Weapon->BaseStatBonus.Speed, State.Weapon->BaseStatBonus.AttackPower);
	}
	for (const TPair<FName, TObjectPtr<UArmorDataAsset>>& ArmorPair : State.Armors) 
	{
		const UArmorDataAsset* Armor = ArmorPair.Value; 
		if (!Armor) continue; 
		Out.MaxHP += Armor->BaseStatBonus.MaxHP; 
		Out.Speed += Armor->BaseStatBonus.Speed; 
		Out.AttackPower += Armor->BaseStatBonus.AttackPower; 
		UE_LOG(LogTemp, Warning, TEXT("[EquipSub]  Armor[%s] bonus: HP=%.1f SPD=%.1f ATK=%.1f"), 
			*ArmorPair.Key.ToString(), Armor->BaseStatBonus.MaxHP, Armor->BaseStatBonus.Speed, Armor->BaseStatBonus.AttackPower); 
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

	// 장착 파츠 스탯 보너스 추가 합산 
	const FWeaponPartEffect PartEffect = GetTotalEquippedPartEffect(PartyIndex); 
	Out.MaxHP = FMath::Max(1.f, Out.MaxHP + PartEffect.BonusMaxHP); 
	Out.Speed = FMath::Max(1.f, Out.Speed + PartEffect.BonusSpeed); 
	Out.AttackPower = FMath::Max(1.f, Out.AttackPower + PartEffect.BonusAttackPower); 
	return Out;
}