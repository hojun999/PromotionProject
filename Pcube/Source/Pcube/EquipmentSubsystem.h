// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponDataAsset.h"
#include "ArmorDataAsset.h"
#include "UnitStatTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WeaponPartDataAsset.h"
#include "EquipmentSubsystem.generated.h"

class UInventorySubsystem;
class UArmorDataAsset;
class UUnitDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, int32, PartyIndex);

USTRUCT()
struct FUnitEquipmentState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UWeaponDataAsset> Weapon = nullptr;

	UPROPERTY()
	TObjectPtr<UArmorDataAsset> Armor = nullptr;

	// PartSlotKey(SlotId) -> Equipped Part
	UPROPERTY()
	TMap<FName, TObjectPtr<UWeaponPartDataAsset>> EquippedParts;
};

UCLASS()
class PCUBE_API UEquipmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnEquipmentChanged OnEquipmentChanged;

	UFUNCTION(BlueprintCallable)
	void EnsurePartySize(int32 NumPartyMembers);

	// 유닛 데이터에 지정된 기본 무기/방어구를 최초 1회만 장착 상태로 동기화
	UFUNCTION(BlueprintCallable)
	void InitializeUnitLoadoutIfMissing(int32 PartyIndex, UUnitDataAsset* UnitData);
	
	// 무기 교체(기존 파츠는 인벤으로 반환 후 초기화하는 단순 정책)
	UFUNCTION(BlueprintCallable)
	bool EquipWeapon(int32 PartyIndex, UWeaponDataAsset* NewWeapon);

	UFUNCTION(BlueprintCallable)
	UWeaponDataAsset* GetEquippedWeapon(int32 PartyIndex) const;

	UFUNCTION(BlueprintCallable)
	UArmorDataAsset* GetEquippedArmor(int32 PartyIndex) const;
	
	// PartSlotKey는 WeaponDataAsset/ArmorDataAsset의 ModSlots[].SlotId를 사용
	UFUNCTION(BlueprintCallable)
	UWeaponPartDataAsset* GetEquippedPart(int32 PartyIndex, FName PartSlotKey) const;

	UFUNCTION(BlueprintCallable)
	bool CanEquipWeaponPart(int32 PartyIndex, FName PartSlotKey, UWeaponPartDataAsset* NewPart) const;

	UFUNCTION(BlueprintCallable)
	bool EquipWeaponPart(int32 PartyIndex, FName PartSlotKey, UWeaponPartDataAsset* NewPart);
	
	UFUNCTION(BlueprintCallable)
	bool UnequipWeaponPart(int32 PartyIndex, FName PartSlotKey);

	UFUNCTION(BlueprintCallable)
	FWeaponPartEffect GetTotalEquippedPartEffect(int32 PartyIndex) const;
	
	// UFUNCTION(BlueprintCallable)
	// bool IsPartCompatibleWithWeapon(UWeaponDataAsset* Weapon, FName SlotSocketName, const UWeaponPartDataAsset* Part) const;

	// 장비 기본 스탯 합산(무기 + 방어구)
	
	// 기존 호출부 호환용. 현재는 무기/방어구 장착 파츠 전체 합산을 반환한다.
	// UFUNCTION(BlueprintCallable)
	// FWeaponPartEffect GetTotalWeaponPartEffect(int32 PartyIndex) const;

	UFUNCTION(BlueprintCallable)
	FEquipmentStatBonus GetTotalEquipmentBaseStatBonus(int32 PartyIndex) const;
	
	UFUNCTION(BlueprintCallable)
	FUnitBaseStats ResolveFinalStats(int32 PartyIndex, const FUnitBaseStats& BaseStats) const;
	
private:
	UPROPERTY()
	TArray<FUnitEquipmentState> PartyEquipments;

	UInventorySubsystem* GetInv() const;
	bool IsValidParty(int32 PartyIndex) const;
};
