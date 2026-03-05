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
	// Sets default values for this actor's properties
	ALootCorpseActor();

	// WorldEnemy가 생성 직후 호출 - 시체 메시/EncounterID/루팅 목록 세팅
	void InitCorpse(FName InEncounterID, UStaticMesh* InCorpseMesh, const TArray<FLootStack>& InLoot); // 시체 초기화
	
	FName GetEncounterID() const { return EncounterID; }
	const TArray<FLootStack>& GetLootItems() const { return LootItems; } // UI가 읽는 루팅 목록
	
	// UI에서 호출
	bool TakeLootAt(int32 Index, FLootStack& OutTaken); // 슬롯 루팅 - 해당 슬롯 아이템의 전체 개수
	
	// UI에서 호출
	void TakeAllLoot(TArray<FLootStack>& OutTakenAll); // 전체 루팅 
	
	bool IsEmpty() const { return LootItems.Num() == 0; } // 남은 루팅 없음
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Overlap로 E 루팅 안내 + 입력 허용
	UFUNCTION()
	void OnInteractBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnInteractEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// E 키로 루팅 창 열기
	void OpenLootUI(); // WorldHUD에 루팅창 열기 요청
	
	// 루팅 배열을 앞에서부터 채워지게 유지 (빈 슬롯 제거)
	void CompactLoot(); // 루팅 목록 압축 (앞으로 당기기)
	
	// TransferSubsystem에 잔여 루팅 저장/완료 처리
	void SaveLootState();
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CorpseMeshComp = nullptr; // 시체 메시
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractBox = nullptr; // 상호작용(Overlap)
	
	UPROPERTY()
	FName EncounterID = NAME_None; // Encounter 키

	UPROPERTY()
	TArray<FLootStack> LootItems; // 잔여 루팅 - 최대 8
	
	UPROPERTY()
	TObjectPtr<UInputComponent> BoundInputComponent = nullptr;
	
	//bool bEKeyBound = false; // 중복 바인딩 금지
	
};
