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

	// 방어구는 여러 개 지원
	UPROPERTY()
	TMap<FName, TObjectPtr<UArmorDataAsset>> Armors;

	// 현재는 부품 슬롯 키(SlotId) -> 장착 파츠로 저장
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
	UArmorDataAsset* GetEquippedArmor(int32 PartyIndex, FName ArmorID) const; // ArmorID로 조회
	
	const TMap<FName, TObjectPtr<UArmorDataAsset>>* GetAllEquippedArmors(int32 PartyIndex) const;

	
	UFUNCTION(BlueprintCallable)
	UWeaponPartDataAsset* GetEquippedPart(int32 PartyIndex, FName PartSlotKeyOrSocketName) const;

	UFUNCTION(BlueprintCallable)
	bool CanEquipWeaponPart(int32 PartyIndex, FName PartSlotKeyOrSocketName, UWeaponPartDataAsset* NewPart) const;

	UFUNCTION(BlueprintCallable)
	bool EquipWeaponPart(int32 PartyIndex, FName PartSlotKeyOrSocketName, UWeaponPartDataAsset* NewPart);
	
	UFUNCTION(BlueprintCallable)
	bool UnequipWeaponPart(int32 PartyIndex, FName PartSlotKeyOrSocketName);

	UFUNCTION(BlueprintCallable)
	FWeaponPartEffect GetTotalEquippedPartEffect(int32 PartyIndex) const;
	
	UFUNCTION(BlueprintCallable)
	FWeaponPartEffect GetTotalWeaponPartEffect(int32 PartyIndex) const { return GetTotalEquippedPartEffect(PartyIndex); }

	UFUNCTION(BlueprintCallable)
	FEquipmentStatBonus GetTotalEquipmentBaseStatBonus(int32 PartyIndex) const;
	
	UFUNCTION(BlueprintCallable)
	FUnitBaseStats ResolveFinalStats(int32 PartyIndex, const FUnitBaseStats& BaseStats) const;
	
private:
	UPROPERTY()
	TArray<FUnitEquipmentState> PartyEquipments;

	UInventorySubsystem* GetInv() const;
	bool IsValidParty(int32 PartyIndex) const;
	const FWeaponModSlotDef* FindWeaponSlotDef(const UWeaponDataAsset* Weapon, FName PartSlotKeyOrSocketName) const;
	const FWeaponModSlotDef* FindArmorSlotDef(const UArmorDataAsset* Armor, FName PartSlotKeyOrSocketName) const;
	FName ResolveStoredPartKey(int32 PartyIndex, FName PartSlotKeyOrSocketName) const;

};
