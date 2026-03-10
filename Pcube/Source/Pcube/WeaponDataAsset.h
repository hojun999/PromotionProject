// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UnitStatTypes.h"
#include "WeaponModTypes.h"
#include "WeaponDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWeaponModSlotDef
{
	GENERATED_BODY()

	// 논리 파츠 슬롯 식별자. CompatibleEquipmentKeys와 직접 매칭된다.
	// 예: KnightSword_Blade, KnightSword_Grip, MageStaff_Core
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot")
	FName SlotId = NAME_None;

	// 레거시/분류용 타입. 현재 호환 판정의 주 기준은 SlotId.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot")
	EWeaponModSlotType SlotType = EWeaponModSlotType::Custom;

	// 실제 메시 소켓이 필요할 때만 사용하는 시각용 데이터.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot|Visual")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot|UI", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D UIAnchor = FVector2D(0.5f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot|UI")
	FVector2D UIPixelOffset = FVector2D(0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot|UI")
	FText DisplayName;
};

USTRUCT(BlueprintType)
struct FWeaponPreviewSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview")
	FTransform WeaponTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview")
	FVector CameraLocation = FVector(200.f, 0.f, 20.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview")
	FRotator CameraRotation = FRotator(0.f, 180.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview")
	float FOV = 30.f;
};

UCLASS(BlueprintType)
class PCUBE_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FName WeaponID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|UI")
	TObjectPtr<UTexture2D> IllustrationTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Stats")
	FEquipmentStatBonus BaseStatBonus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<UStaticMesh> WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Parts")
	TArray<FWeaponModSlotDef> ModSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Preview")
	FWeaponPreviewSettings Preview;
};
