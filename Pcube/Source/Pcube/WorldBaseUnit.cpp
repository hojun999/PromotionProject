// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldBaseUnit.h"
#include "UnitDataAsset.h"
#include "EquipmentSubsystem.h"
#include "WeaponDataAsset.h"
#include "WeaponPartDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AWorldBaseUnit::AWorldBaseUnit()
{
	PrimaryActorTick.bCanEverTick = false;

	// 월드 이동 유닛이므로 Movement 기본값
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
		MoveComp->MaxWalkSpeed = 300.f;
	}
	
	// 컨트롤러 회전 영향 제거
	bUseControllerRotationYaw = false;
	
	// ACharacter 기본 메시 정렬은 BP에서 조정하는게 가장 편하다고 함
	// 근데 내 구조에서 BP에서 조정하는게 가능한가?

	// 무기/파츠 비주얼용 컴포넌트
	WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComp"));
	WeaponMeshComp->SetupAttachment(GetMesh());
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetGenerateOverlapEvents(false);
	WeaponMeshComp->SetHiddenInGame(true);
}

void AWorldBaseUnit::BeginPlay()
{
	Super::BeginPlay();

	if (PartyIndex == INDEX_NONE)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
		{
			EquipSub->EnsurePartySize(PartyIndex + 1);
			EquipSub->OnEquipmentChanged.RemoveDynamic(this, &AWorldBaseUnit::HandleEquipmentChanged);
			EquipSub->OnEquipmentChanged.AddDynamic(this, &AWorldBaseUnit::HandleEquipmentChanged);
		}
	}

	RefreshEquipmentVisuals();
}

void AWorldBaseUnit::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
		{
			EquipSub->OnEquipmentChanged.RemoveDynamic(this, &AWorldBaseUnit::HandleEquipmentChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AWorldBaseUnit::HandleEquipmentChanged(int32 ChangedPartyIndex)
{
	if (PartyIndex == INDEX_NONE) return;
	if (ChangedPartyIndex != PartyIndex) return;
	RefreshEquipmentVisuals();
}

void AWorldBaseUnit::RefreshEquipmentVisuals()
{
	if (!WeaponMeshComp) return;

	if (PartyIndex == INDEX_NONE)
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
		WeaponMeshComp->SetHiddenInGame(true);
		for (auto& KVP : PartMeshBySocket)
		{
			if (KVP.Value)
			{
				KVP.Value->DestroyComponent();
			}
		}
		PartMeshBySocket.Empty();
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UEquipmentSubsystem* EquipSub = GI ? GI->GetSubsystem<UEquipmentSubsystem>() : nullptr;
	if (!EquipSub) return;

	UWeaponDataAsset* WeaponDA = EquipSub->GetEquippedWeapon(PartyIndex);
	if (WeaponDA && WeaponDA->WeaponMesh)
	{
		WeaponMeshComp->SetStaticMesh(WeaponDA->WeaponMesh);
		WeaponMeshComp->SetHiddenInGame(false);
	}
	else
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
		WeaponMeshComp->SetHiddenInGame(true);
	}

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		if (WeaponHoldSocketName != NAME_None && Skel->DoesSocketExist(WeaponHoldSocketName))
		{
			WeaponMeshComp->AttachToComponent(Skel, FAttachmentTransformRules::KeepRelativeTransform, WeaponHoldSocketName);
		}
		else
		{
			WeaponMeshComp->AttachToComponent(Skel, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	TSet<FName> DesiredSockets;
	if (WeaponDA)
	{
		for (const FWeaponModSlotDef& Def : WeaponDA->ModSlots)
		{
			if (Def.SocketName != NAME_None)
			{
				DesiredSockets.Add(Def.SocketName);
			}
		}
	}

	for (auto It = PartMeshBySocket.CreateIterator(); It; ++It)
	{
		if (!DesiredSockets.Contains(It.Key()))
		{
			if (It.Value())
			{
				It.Value()->DestroyComponent();
			}
			It.RemoveCurrent();
		}
	}

	for (const FName SocketName : DesiredSockets)
	{
		UWeaponPartDataAsset* PartDA = EquipSub->GetEquippedPart(PartyIndex, SocketName);
		UStaticMeshComponent* PartComp = PartMeshBySocket.FindRef(SocketName);

		if (!PartDA || !PartDA->AttachmentMesh || WeaponMeshComp->GetStaticMesh() == nullptr)
		{
			if (PartComp)
			{
				PartComp->DestroyComponent();
				PartMeshBySocket.Remove(SocketName);
			}
			continue;
		}

		if (!WeaponMeshComp->DoesSocketExist(SocketName))
		{
			UE_LOG(LogTemp, Warning, TEXT("[EquipmentVisual][World] Weapon mesh socket not found: %s (Weapon=%s Unit=%s)"),
				*SocketName.ToString(), *GetNameSafe(WeaponDA), *GetName());
			continue;
		}

	if (!PartComp)
		{
			PartComp = NewObject<UStaticMeshComponent>(this);
			if (!PartComp) continue;
			PartComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PartComp->SetGenerateOverlapEvents(false);
			PartComp->RegisterComponent();
			PartComp->AttachToComponent(WeaponMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			PartMeshBySocket.Add(SocketName, PartComp);
		}

		PartComp->SetStaticMesh(PartDA->AttachmentMesh);
		PartComp->SetHiddenInGame(false);
	}
}

void AWorldBaseUnit::InitFromUnitData(UUnitDataAsset* InUnitData)
{
	UnitData = InUnitData;
	if (!UnitData)
	{
		UE_LOG(LogTemp, Error, TEXT("[WorldBaseUnit] UnitData is null"));
		return;
	}
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[WorldBaseUnit] GetMesh() is null"));
		return;
	}
	
	// 월드 스켈레탈 적용
	if (UnitData->WorldSkeletalMesh)
	{
		MeshComp->SetSkeletalMesh(UnitData->WorldSkeletalMesh);
		MeshComp->SetHiddenInGame(false);
	}
	
	// 월드 애님 BP 적용
	if (UnitData->WorldAnimBlueprintClass)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(UnitData->WorldAnimBlueprintClass);
	}
	else
	{
		// 프로토타입 - 애님BP 없으면 포즈 고정
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[WorldBaseUnit] Init OK: %s Skeletal=%s AnimBP=%s"),
		*GetName(),
		*GetNameSafe(UnitData->WorldSkeletalMesh),
		*GetNameSafe(UnitData->WorldAnimBlueprintClass));
}
