// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WeaponPartDataAsset.h"
#include "EquipmentSubsystem.generated.h"

class UInventorySubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, int32, PartyIndex);

USTRUCT()
struct FUnitWeaponState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UWeaponDataAsset> Weapon = nullptr;

	// SocketName -> Equipped Part
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

	// 무기 교체(기존 파츠는 인벤으로 반환 후 초기화하는 단순 정책)
	UFUNCTION(BlueprintCallable)
	bool EquipWeapon(int32 PartyIndex, UWeaponDataAsset* NewWeapon);

	UFUNCTION(BlueprintCallable)
	UWeaponDataAsset* GetEquippedWeapon(int32 PartyIndex) const;

	UFUNCTION(BlueprintCallable)
	UWeaponPartDataAsset* GetEquippedPart(int32 PartyIndex, FName SlotSocketName) const;

	UFUNCTION(BlueprintCallable)
	bool EquipWeaponPart(int32 PartyIndex, FName SlotSocketName, UWeaponPartDataAsset* NewPart);

	UFUNCTION(BlueprintCallable)
	bool UnequipWeaponPart(int32 PartyIndex, FName SlotSocketName);

	// 합산 효과(전투/스탯 패널용)
	UFUNCTION(BlueprintCallable)
	FWeaponPartEffect GetTotalWeaponPartEffect(int32 PartyIndex) const;

private:
	UPROPERTY()
	TArray<FUnitWeaponState> PartyWeapons;

	UInventorySubsystem* GetInv() const;
	bool IsValidParty(int32 PartyIndex) const;
	const FWeaponModSlotDef* FindSlotDef(UWeaponDataAsset* Weapon, FName SlotSocketName) const;
};