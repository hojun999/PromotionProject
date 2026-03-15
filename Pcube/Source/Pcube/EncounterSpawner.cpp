// Fill out your copyright notice in the Description page of Project Settings.


#include "EncounterSpawner.h"

#include "BattleInfoTransferSubsystem.h"
#include "WorldEnemyUnit.h"

// Sets default values
AEncounterSpawner::AEncounterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEncounterSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!WorldEnemyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[EncounterSpawner] WorldEnemyClass missing: %s"), *GetName());
		return;
	}
	
	UGameInstance* GI = GetWorld()->GetGameInstance();
	UBattleInfoTransferSubsystem* Transfer = GI ? GI->GetSubsystem<UBattleInfoTransferSubsystem>() : nullptr;
	
	const bool bDefeated = (Transfer && EncounterID != NAME_None)
		? Transfer->IsEncounterDefeated(EncounterID)
		: false;
	
	if (bDefeated && bSkipSpawnIfDefeated)
	{
		return;
	}
	
	UUnitDataAsset* UnitData = UnitDataAsset.IsNull() ? nullptr : UnitDataAsset.LoadSynchronous();
	USpawnDataAsset* SpawnData = SpawnDataAsset.IsNull() ? nullptr : SpawnDataAsset.LoadSynchronous();
	
	if (!UnitData || !SpawnData)
	{
		UE_LOG(LogTemp, Error, TEXT("[EncounterSpawner] DataAsset missing: %s"), *GetName());
		return;
	}
	
	const FTransform SpawnTM = GetActorTransform();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	
	// *** 이 스포너가 속한 서브레벨에 적을 스폰해서
	// 서브레벨 언로드 시 같이 사라지게 설정
	SpawnParams.OverrideLevel = GetLevel(); // OverrideLevel은 스폰될 Actor의 Outer(Level) 지정
	
	// *** 디퍼로 설정하여 BeginPlay 이전에 EncounterID/데이터 할당
	SpawnParams.bDeferConstruction = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AWorldEnemyUnit* Enemy = GetWorld()->SpawnActor<AWorldEnemyUnit>(WorldEnemyClass, SpawnTM, SpawnParams);
	if (!Enemy) return;
	
	Enemy->InitializeEncounterInfo(EncounterID, UnitData, SpawnData, bDefeated);
	Enemy->SetPatrolPoints(PatrolPoints); // 순찰 포인트 전달
	Enemy->SetBossEncounter(bIsBossEncounter); // 보스 여부 전달
	Enemy->FinishSpawning(SpawnTM);
}



