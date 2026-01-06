// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldEnemyUnit.h"
#include "GlobalDataInstance.h"
#include "UnitDataAsset.h"
#include "WorldAllyUnit.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AWorldEnemyUnit::AWorldEnemyUnit()
{
	DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
	DetectSphere->SetupAttachment(RootComponent);
	DetectSphere->SetSphereRadius(600.f);	// 감지 범위 설정
}

void AWorldEnemyUnit::BeginPlay()
{
	Super::BeginPlay();
	
	// 감지 이벤트 바인딩
	if (DetectSphere)
	{
		DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
	}
}

void AWorldEnemyUnit::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
									  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
									  bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		UE_LOG(LogTemp, Warning, TEXT("Detected Actor: %s"), *OtherActor->GetName());
		// TODO: 추격 AI 로직을 작성, AI controller를 사용하여 Player를 향해 MoveTo 실행
	}
}

void AWorldEnemyUnit::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	// 플레이어와 부딪혔을 때 인카운터 발생
	if (OtherActor->IsA(AWorldAllyUnit::StaticClass()))
	{
		StartEncounter(OtherActor);
	}
}

void AWorldEnemyUnit::StartEncounter(AActor* PlayerActor)
{
	UGlobalDataInstance* GI = Cast<UGlobalDataInstance>(GetGameInstance());
	AWorldAllyUnit* PlayerUnit = Cast<AWorldAllyUnit>(PlayerActor);
	
	if (GI && PlayerUnit)
	{
		// 플레이어 데이터와 Enemy 데이터를 GameInstance에 저장
		GI->BattleInfo.EnemyClasses.Empty();
		GI->BattleInfo.EnemyClasses.Add(UnitData->BattleUnitClass);
		GI->BattleInfo.SourceLevelName = FName(*GetWorld()->GetMapName());
		GI->BattleInfo.ReturnLocation = PlayerActor->GetActorLocation();
		
		UGameplayStatics::OpenLevel(this, FName("BattleLevel"));
	}
}

