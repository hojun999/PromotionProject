// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponModTypes.h"
#include "WeaponDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWeaponModSlotDef
{
	GENERATED_BODY()

	// 저장/조회용 고정 ID (예: "Muzzle", "GripA", "Rail_Left")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot")
	FName SlotId = NAME_None;

	// 호환 타입(필터 기준)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot")
	EWeaponModSlotType SlotType = EWeaponModSlotType::Custom;

	// 실제 무기 메시의 소켓 이름(부착 위치)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot")
	FName SocketName = NAME_None;

	// UI 상에 슬롯 버튼을 배치할 위치 (0~1 정규화 좌표)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot|UI", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D UIAnchor = FVector2D(0.5f, 0.5f);

	// 미세 조정 픽셀 오프셋
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

	// 무기 본체 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<UStaticMesh> WeaponMesh = nullptr;

	// 이 무기의 모딩 슬롯 정의(개수/소켓/UI 위치 전부 여기서 결정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Mod")
	TArray<FWeaponModSlotDef> ModSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Preview")
	FWeaponPreviewSettings Preview;
};
