#pragma once

#include "CoreMinimal.h"
#include "WeaponModTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponModSlotType : uint8
{
	Barrel      UMETA(DisplayName="Barrel"),
	Grip        UMETA(DisplayName="Grip"),
	Bar			UMETA(DisplayName="Bar"),
	Jewel       UMETA(DisplayName="Jewel"),
	Custom		UMETA(DisplayName="Custom"),
};