// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WorldBaseUnit.generated.h"

class UUnitDataAsset;
class UStaticMeshComponent;

UCLASS()
class PCUBE_API AWorldBaseUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWorldBaseUnit();
	
	UFUNCTION(BlueprintCallable)
	virtual void InitFromUnitData(UUnitDataAsset* InUnitData);

	// --- 장비(무기/파츠) 비주얼 ---
	// 월드에서 이 유닛이 어떤 파티원(PartyIndex)에 대응되는지.
	// PartyIndex가 INDEX_NONE이면 장비 비주얼을 갱신하지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment")
	int32 PartyIndex = INDEX_NONE;

	// 무기 본체 메시(WeaponDataAsset->WeaponMesh)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment|Visual")
	TObjectPtr<UStaticMeshComponent> WeaponMeshComp = nullptr;

	// 캐릭터 메시(손 등)에 무기를 붙일 소켓. 비워두면 Mesh Root에 붙음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment|Visual")
	FName WeaponHoldSocketName = NAME_None;

	UFUNCTION(BlueprintCallable, Category="Equipment|Visual")
	void RefreshEquipmentVisuals();
	
protected:
	// 유닛의 정보를 담은 데이터 에셋 (월드 - 전투 연결을 위함)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UUnitDataAsset> UnitData = nullptr;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleEquipmentChanged(int32 ChangedPartyIndex);

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UStaticMeshComponent>> PartMeshBySocket;
	
public:	
	

};
