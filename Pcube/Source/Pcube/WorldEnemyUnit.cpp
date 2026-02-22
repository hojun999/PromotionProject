// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldEnemyUnit.h"
#include "BattleInfoTransferSubsystem.h"
#include "SpawnDataAsset.h"
#include "UnitDataAsset.h"
#include "WorldAllyUnit.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	
	// 시체 메시는 캡슐(RootComponent)에 부착
	DeadStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeadStaticMeshComp"));
	DeadStaticMeshComp->SetupAttachment(RootComponent);
	
	DeadStaticMeshComp->SetHiddenInGame(true);
	DeadStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWorldEnemyUnit::BeginPlay()
{
	Super::BeginPlay();
	
	
	// 로드 직후 즉시 오버랩 전부 꺼두기
	if (DetectSphere)
	{
		DetectSphere->SetGenerateOverlapEvents(false);
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetectSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
	}

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetGenerateOverlapEvents(false);
		Cap->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnEncounterOverlap);
		Cap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	// defeated 체크
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UBattleInfoTransferSubsystem* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>())
		{
			const bool bDefeated = Transfer->IsEncounterDefeated(EncounterID);
			
			UE_LOG(LogTemp, Warning, TEXT("[WorldEnemy] BeginPlay %s EncounterID=%s defeated=%d"),
				*GetName(), *EncounterID.ToString(), bDefeated ? 1 : 0);
			
			if (bDefeated)
			{
				ConvertToCorpse();
				return; // ★ 여기서 끝내야 함
			}
		}
	}
	
	// 살아있을 때만 오버랩 키고 바인딩
	if (DetectSphere)
	{
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DetectSphere->SetGenerateOverlapEvents(true);
		DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
	}

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetGenerateOverlapEvents(true);
		Cap->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnEncounterOverlap);
		// Pawn 오버랩만 허용(기존 설정 유지)
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

void AWorldEnemyUnit::InitFromUnitData(UUnitDataAsset* InUnitData)
{
	Super::InitFromUnitData(InUnitData);
	
	if (!UnitData || !DeadStaticMeshComp) return;
	
	if (UnitData->DeadStaticMesh)
	{
		DeadStaticMeshComp->SetStaticMesh(UnitData->DeadStaticMesh);
	}
	
	DeadStaticMeshComp->SetHiddenInGame(true);
}

void AWorldEnemyUnit::InitializeEncounterInfo(FName InEncounterID, UUnitDataAsset* InUnitData, USpawnDataAsset* InSpawnData, bool bAlreadyDefeated)
{
	EncounterID = InEncounterID;
	SpawnData = InSpawnData;
	
	InitFromUnitData(InUnitData);
	
	if (bAlreadyDefeated)
	{
		ConvertToCorpse();
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
	// 시체의 경우 전투 재시작 방지
	if (bIsCorpse || bEncounterLocked) return;
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>())
		{
			if (Transfer->IsEncounterDefeated(EncounterID))
			{
				return;
			}
		}
	}
	
	if (OtherActor && OtherActor->IsA(AWorldAllyUnit::StaticClass()))
	{
		StartEncounter(OtherActor);
	}
}


void AWorldEnemyUnit::StartEncounter(AActor* PlayerActor)
{
	UE_LOG(LogTemp, Warning, TEXT("[WorldEnemy] StartEncounter name=%s id=%s locked=%d corpse=%d"),
	*GetName(), *EncounterID.ToString(), bEncounterLocked?1:0, bIsCorpse?1:0);
	
	if (bIsCorpse || bEncounterLocked) return;
	bEncounterLocked = true;
	
	if (!SpawnData  || !UnitData)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 데이터 에셋 할당 확인 필요!"), *GetName());
		return;
	}
	
	// 오버랩이 또 들어와도 전투 재호출 안 되게 즉시 차단
	if (DetectSphere)
	{
		DetectSphere->SetGenerateOverlapEvents(false);
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetGenerateOverlapEvents(false);
		Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UBattleInfoTransferSubsystem* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	if (!Transfer) return;
	
	if (EncounterID == NAME_None)
	{
		UE_LOG(LogTemp, Error, TEXT("[WorldEnemy] EncounterID is None. Defeat tracking will fail."));
	}
	
	// 1. ReturnWorldLevel 결정
	const FName WorldLevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	
	// 2. PendingEncounter 세팅
	Transfer->SetPendingEncounter(EncounterID, WorldLevelName);
	
	// 3. 월드 복귀 위치 저장 (Consume 대상)
	Transfer->SetReturnPoint(WorldLevelName, PlayerActor->GetActorLocation(), PlayerActor->GetActorRotation());
	
	// 4. 적 스폰 정보 저장
	if (SpawnData->SpawnGroups.Num() > 0)
	{
		Transfer->InitEnemyBattleInfo(SpawnData->SpawnGroups[0].EnemyList);
	}

	// 5. 전투 레벨 이동
	UGameplayStatics::OpenLevel(this, FName("L_Battle"));
}

void AWorldEnemyUnit::ConvertToCorpse()
{
	if (bIsCorpse) return;
	bIsCorpse = true;
	
	// 1. 감지 스피어 비활성화
	if (DetectSphere)
	{
		DetectSphere->SetGenerateOverlapEvents(false);
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetectSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
	}
	
	// 2. 전투 트리거 비활성화
	if (UCapsuleComponent* CapComp = GetCapsuleComponent())
	{
		CapComp->SetGenerateOverlapEvents(false);
		CapComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CapComp->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnEncounterOverlap);
	}
	
	// 3. 이동/AI 정지
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	
	// 4. 살아있을 때 쓰던 메시 끄기
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetHiddenInGame(true);
		MeshComp->SetComponentTickEnabled(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// 5. 시체 메시 켜기
	if (DeadStaticMeshComp)
	{
		DeadStaticMeshComp->SetHiddenInGame(false);
		
		// 루팅/클릭 상호작용할 거면 QueryOnly로
		DeadStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DeadStaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		DeadStaticMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
	
	// 6. 상호작용 박스 설정
	if (!InteractBox)
	{
		InteractBox = NewObject<UBoxComponent>(this, TEXT("InteractBox"));
		InteractBox->RegisterComponent();
		InteractBox->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		InteractBox->SetBoxExtent(FVector(120, 120, 120));
		InteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		InteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		InteractBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnCorpseBeginOverlap);
		InteractBox->OnComponentEndOverlap.AddDynamic(this, &AWorldEnemyUnit::OnCorpseEndOverlap);
	}	
}

void AWorldEnemyUnit::OnCorpseBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;
	
	EnableInput(PC);
	
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AWorldEnemyUnit::Loot);
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Press E to Loot"));
	}
}


void AWorldEnemyUnit::OnCorpseEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;
	
	DisableInput(PC);
}

void AWorldEnemyUnit::Loot()
{
	if (!bIsCorpse) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[Loot] Player looted corpse: %s"), *GetName());
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("[Loot] Items acquired (prototype)"));
	}
	
	// 프로토타입: 루팅 후 제거
	Destroy();
}
