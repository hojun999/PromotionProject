// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldEnemyUnit.h"
#include "GlobalDataInstance.h"
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
	if (!EncounterData)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 에 EncounterDataAsset이 할당되지 않았습니다!"), *GetName());
		return;
	}
	
	UGlobalDataInstance* GI = Cast<UGlobalDataInstance>(GetGameInstance());
	AWorldAllyUnit* PlayerUnit = Cast<AWorldAllyUnit>(PlayerActor);
	
	if (UnitData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy [%s] has NO UnitData! 할당을 확인하세요."), *GetName());
		return;
	}
	
	if (GI && PlayerUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("Encounter Started! Loading Battle Level..."));
		
		//GI->BattleInfo.EnemyClasses = EncounterData->EnemyGroup;
		
		// 플레이어 데이터와 Enemy 데이터를 GameInstance에 저장
		GI->BattleInfo.EnemyClasses.Empty();
		GI->BattleInfo.EnemyClasses.Add(UnitData->BattleUnitClass);
		GI->BattleInfo.SourceLevelName = FName(*GetWorld()->GetMapName());
		GI->BattleInfo.ReturnLocation = PlayerActor->GetActorLocation();
		GI->BattleInfo.ReturnRotation = PlayerActor->GetActorRotation();
		
		UGameplayStatics::OpenLevel(this, FName("BattleLevel"));
	}
}

