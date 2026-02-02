// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleEnemyUnit.h"

#include "UnitDataAsset.h"

ABattleEnemyUnit::ABattleEnemyUnit()
{
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	
	// ACharacter를 상속받으므로 깁본 skeletalmesh 숨기기
	// TODO: 이후에 Skeletalmesh 사용할 때 아래 내용 삭제
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
	}
}

void ABattleEnemyUnit::Die()
{
	// DropTable 에 등록된 모든 재화에 대해 루프 실행
	for (const FDropInfo& Drop : DropTable)
	{
		if (Drop.ItemName != NAME_None)
		{
			int32 DroppedAmount = FMath::RandRange(Drop.MinAmount, Drop.MaxAmount);
			UE_LOG(LogTemp, Warning, TEXT("%s가 %s를 %d개 드롭*****"), 
				*GetName(), *Drop.ItemName.ToString(), DroppedAmount);
			
			// 실제 인벤토리/재화 시스템에 재화(DroppedAmount 값) 전달 
			// -> 아마 시체에 상호작용하는 방식으로 전달 or 바로 인벤토리로 전달하면 UI로 몇 개 획득했는지만 띄울듯 (시체는 구현할 것)
		}
	}
	
	Super::Die();
}
