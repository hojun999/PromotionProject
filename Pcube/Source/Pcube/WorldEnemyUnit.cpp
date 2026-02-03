// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldEnemyUnit.h"
#include "BattleInfoTransferSubsystem.h"
#include "SpawnDataAsset.h"
#include "UnitDataAsset.h"
#include "WorldAllyUnit.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
//#include "Blueprint/AIBlueprintHelperLibrary.h"

AWorldEnemyUnit::AWorldEnemyUnit()
{
	// 감지용 collision
	DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
	DetectSphere->SetupAttachment(RootComponent);
	DetectSphere->SetSphereRadius(400.f);	// 감지 범위 설정
	//  감지 component는 물리적 충돌 없이 overlap만 판정
	DetectSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// 본체 collision (전투 진입-레벨 전환 범위)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AWorldEnemyUnit::BeginPlay()
{
	Super::BeginPlay();
	
	// 감지 이벤트 바인딩
	if (DetectSphere)
	{
		DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
	}
	
	// 전투 시작 이벤트 바인딩
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnEncounterOverlap);
	}
}

// 플레이어 발견 시 로직 - sphere에 닿은 경우
void AWorldEnemyUnit::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
									  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
									  bool bFromSweep, const FHitResult& SweepResult)
{
	AWorldAllyUnit* Player = Cast<AWorldAllyUnit>(OtherActor);
	
	if (Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 감지: %s"), *Player->GetName());
		
		// (아래 로직 이후에 수정 가능성 있음) TODO: 추격 AI 로직을 작성, AI controller를 사용하여 Player를 향해 MoveTo 실행
		// if (GetController())
		// {
		// 	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), Player);
		// }
	}
}

// 플레이어와 충돌 시 로직 - capsule에 닿은 경우
void AWorldEnemyUnit::OnEncounterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
										UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
										bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AWorldAllyUnit::StaticClass()))
	{
		StartEncounter(OtherActor);
	}
}


void AWorldEnemyUnit::StartEncounter(AActor* PlayerActor)
{
	if (!SpawnData  || !UnitData)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 데이터 에셋 할당 확인 필요!"), *GetName());
		return;
	}
	
	// GameInstance에서 인스턴스 서브시스템 가져오기
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}
	
	UBattleInfoTransferSubsystem* BattleInfoSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	AWorldAllyUnit* PlayerUnit = Cast<AWorldAllyUnit>(PlayerActor);
	
	if (BattleInfoSubsystem && PlayerUnit)
	{
		UE_LOG(LogTemp, Log, TEXT("Encounter Started! Transferring data via Subsystem..."));
		
		// 인스턴스 서브시스템에 데이터 저장
		if (SpawnData->SpawnGroups.Num() > 0)
		{
			// 이전 전투 정보 삭제 & 새로운 적 스폰 정보 갱신
			BattleInfoSubsystem->InitEnemyBattleInfo(SpawnData->SpawnGroups[0].EnemyList);
			//BattleInfoSubsystem->BattleInfo.EnemiesToSpawn = SpawnData->SpawnGroups[0].EnemyList;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s]의 SpawnGroups가 비어있습니다!"), *SpawnData->GetName());
			return;
		}
	}
	
	// 월드 복귀용 데이터 저장
	BattleInfoSubsystem->BattleInfo.SourceLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this));
	BattleInfoSubsystem->BattleInfo.ReturnLocation = PlayerActor->GetActorLocation();
	BattleInfoSubsystem->BattleInfo.ReturnRotation  = PlayerActor->GetActorRotation();
	
	// 레벨 전환
	UGameplayStatics::OpenLevel(this, FName("BattleLevel"));
}

