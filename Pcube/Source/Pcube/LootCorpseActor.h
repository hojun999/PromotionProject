// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootTypes.h"
#include "Components/BoxComponent.h"
#include "LootCorpseActor.generated.h"

UCLASS()
class PCUBE_API ALootCorpseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALootCorpseActor();

	// WorldEnemy가 생성 직후 호출 - 시체 메시/EncounterID/루팅 목록 세팅
	void InitCorpse(FName InEncounterID, UStaticMesh* InCorpseMesh, const TArray<FLootStack>& InLoot);
	
	FName GetEncounterID() const { return EncounterID; }
	const TArray<FLootStack>& GetLootItems() const { return LootItems; }

	// 특정 슬롯에서 Count만큼 제거한다. 실제로 제거된 수량을 반환한다.
	int32 RemoveLootCountAt(int32 Index, int32 Count, FLootStack* OutRemoved = nullptr);
	
	bool IsEmpty() const { return LootItems.Num() == 0; }
	
protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnInteractBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnInteractEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void OpenLootUI();
	void CompactLoot();
	void SaveLootState();
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CorpseMeshComp = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractBox = nullptr;
	
	UPROPERTY()
	FName EncounterID = NAME_None;

	UPROPERTY()
	TArray<FLootStack> LootItems;
	
	UPROPERTY()
	TObjectPtr<UInputComponent> BoundInputComponent = nullptr;
};
