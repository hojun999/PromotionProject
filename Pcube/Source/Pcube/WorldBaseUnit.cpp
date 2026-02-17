// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldBaseUnit.h"
#include "UnitDataAsset.h"
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
