#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UnitStatTypes.h"
#include "WeaponDataAsset.h"
#include "ArmorDataAsset.generated.h"

UCLASS(BlueprintType)
class PCUBE_API UArmorDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor")
	FName ArmorID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor|UI")
	TObjectPtr<UTexture2D> IllustrationTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor|Stats")
	FEquipmentStatBonus BaseStatBonus;

	// 방어구도 무기와 동일한 파츠 슬롯 레이아웃을 사용한다.
	// SlotId를 CompatibleEquipmentKeys의 키로 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor|Parts")
	TArray<FWeaponModSlotDef> ModSlots;
};
