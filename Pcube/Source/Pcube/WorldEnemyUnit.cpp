// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldEnemyUnit.h"
#include "WorldHUD.h"
#include "BattleInfoTransferSubsystem.h"
#include "SpawnDataAsset.h"
#include "UnitDataAsset.h"
#include "WorldAllyUnit.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

AWorldEnemyUnit::AWorldEnemyUnit()
{
	PrimaryActorTick.bCanEverTick = true; // Tick 활성화

	// 감지용 collision
	DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
	DetectSphere->SetupAttachment(RootComponent);
	DetectSphere->SetSphereRadius(400.f);
	DetectSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 본체 collision (전투 진입 범위)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 시체 메시
	DeadStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeadStaticMeshComp"));
	DeadStaticMeshComp->SetupAttachment(RootComponent);
	DeadStaticMeshComp->SetHiddenInGame(true);
	DeadStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWorldEnemyUnit::BeginPlay()
{
	Super::BeginPlay();

	// 로드 직후 오버랩 전부 꺼두기
	if (DetectSphere)
	{
		DetectSphere->SetGenerateOverlapEvents(false);
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetectSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
		DetectSphere->OnComponentEndOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnDetectEndOverlap);
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
				if (TrySpawnCorpseIfDefeated())
				{
					return;
				}
			}
		}
	}

	// 살아있을 때만 오버랩 + 순찰 시작
	if (DetectSphere)
	{
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DetectSphere->SetGenerateOverlapEvents(true);
		DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
		DetectSphere->OnComponentEndOverlap.AddDynamic(this, &AWorldEnemyUnit::OnDetectEndOverlap);
	}

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetGenerateOverlapEvents(true);
		Cap->OnComponentBeginOverlap.AddDynamic(this, &AWorldEnemyUnit::OnEncounterOverlap);
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	// 0번 인덱스에서 시작
	CurrentPatrolIndex = 0;
	PatrolDirection = 1;
	SetAIState(PatrolPoints.Num() > 0 ? EEnemyAIState::Patrol : EEnemyAIState::Idle);
}

void AWorldEnemyUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCorpse || bEncounterLocked) return;

	switch (AIState)
	{
	case EEnemyAIState::Patrol:
		UpdatePatrol(DeltaTime);
		break;
	case EEnemyAIState::Chase:
		UpdateChase(DeltaTime);
		break;
	case EEnemyAIState::Idle:
	default:
		break;
	}
}

// ── AI 상태 변경 ──────────────────────────────────────────────

void AWorldEnemyUnit::SetAIState(EEnemyAIState NewState)
{
	if (AIState == NewState) return;
	AIState = NewState;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		switch (NewState)
		{
		case EEnemyAIState::Patrol:
			MoveComp->MaxWalkSpeed = PatrolSpeed;
			MoveToNextPatrolPoint(); // 상태 전환 즉시 이동 시작
			break;
		case EEnemyAIState::Chase:
			MoveComp->MaxWalkSpeed = ChaseSpeed;
			break;
		case EEnemyAIState::Idle:
			MoveComp->StopMovementImmediately();
			break;
		}
	}
}

void AWorldEnemyUnit::MoveToNextPatrolPoint()
{
	if (PatrolPoints.Num() == 0) return;
	AAIController* Ctrl = Cast<AAIController>(GetController());
	if (!Ctrl) return;

	// 이동 완료 델리게이트 바인딩
	Ctrl->ReceiveMoveCompleted.RemoveDynamic(this, &AWorldEnemyUnit::OnPatrolMoveCompleted);
	Ctrl->ReceiveMoveCompleted.AddDynamic(this, &AWorldEnemyUnit::OnPatrolMoveCompleted);
	bWaitingForPatrolMove = true;

	FVector Dest = PatrolPoints[CurrentPatrolIndex];

	// 입력된 Z 좌표와 무관하게 NavMesh 위 실제 바닥 위치로 보정
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation ProjectedLoc;
		if (NavSys->ProjectPointToNavigation(Dest, ProjectedLoc, FVector(50.f, 50.f, 500.f)))
		{
			Dest = ProjectedLoc.Location;
		}
	}

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(Ctrl, Dest);
}

void AWorldEnemyUnit::OnPatrolMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	bWaitingForPatrolMove = false;

	if (AIState != EEnemyAIState::Patrol) return;
	if (bIsCorpse || bEncounterLocked) return;
	if (Result != EPathFollowingResult::Success) return;

	const int32 NextIndex = CurrentPatrolIndex + PatrolDirection;
	if (NextIndex < 0 || NextIndex >= PatrolPoints.Num())
	{
		PatrolDirection *= -1;
	}
	const int32 NewIndex = FMath::Clamp(CurrentPatrolIndex + PatrolDirection, 0, PatrolPoints.Num() - 1);
	if (NewIndex != CurrentPatrolIndex)
	{
		CurrentPatrolIndex = NewIndex;
		MoveToNextPatrolPoint();
	}
}

// ── 순찰 ──────────────────────────────────────────────────────

void AWorldEnemyUnit::UpdatePatrol(float DeltaTime)
{
	if (bPatrolPaused) return;
	
	if (PatrolPoints.Num() == 0)
	{
		SetAIState(EEnemyAIState::Idle);
		return;
	}

	// OnPatrolMoveCompleted 델리게이트로 도착 처리
	// 이동 명령이 없는 상태면 (시작 직후 등) 다시 명령
	if (!bWaitingForPatrolMove)
	{
		MoveToNextPatrolPoint();
	}
}

// ── 추격 ──────────────────────────────────────────────────────

void AWorldEnemyUnit::UpdateChase(float DeltaTime)
{
	if (bPatrolPaused) return; // Pause 중 추격 금지
	
	AActor* Target = ChaseTarget.Get();

	if (!IsValid(Target))
	{
		ChaseTarget = nullptr;
		CurrentPatrolIndex = 0; // 순찰 복귀 시 0번부터 재시작
		PatrolDirection = 1;
		SetAIState(PatrolPoints.Num() > 0 ? EEnemyAIState::Patrol : EEnemyAIState::Idle);
		return;
	}

	// 추격은 매 틱 위치가 바뀌므로 지속적으로 이동 명령 갱신
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), Target->GetActorLocation());
}

// ── 감지 이벤트 ───────────────────────────────────────────────

void AWorldEnemyUnit::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
									   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									   bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsCorpse || bEncounterLocked) return;

	AWorldAllyUnit* Player = Cast<AWorldAllyUnit>(OtherActor);
	if (!Player) return;

	UE_LOG(LogTemp, Warning, TEXT("[WorldEnemy] Detected player: %s -> Chase"), *Player->GetName());

	ChaseTarget = Player;
	SetAIState(EEnemyAIState::Chase);
}

void AWorldEnemyUnit::OnDetectEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
										  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == ChaseTarget.Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("[WorldEnemy] Player left detect range -> Patrol"));
		ChaseTarget = nullptr;
		CurrentPatrolIndex = 0; // 순찰 복귀 시 0번부터 재시작
		PatrolDirection = 1;
		SetAIState(PatrolPoints.Num() > 0 ? EEnemyAIState::Patrol : EEnemyAIState::Idle);
	}
}

// ── 인카운터 ──────────────────────────────────────────────────

void AWorldEnemyUnit::OnEncounterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
										  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
										  bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsCorpse || bEncounterLocked) return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>())
		{
			if (Transfer->IsEncounterDefeated(EncounterID)) return;
		}
	}

	if (OtherActor && OtherActor->IsA(AWorldAllyUnit::StaticClass()))
	{
		StartEncounter(OtherActor);
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

void AWorldEnemyUnit::StartEncounter(AActor* PlayerActor)
{
	UE_LOG(LogTemp, Warning, TEXT("[WorldEnemy] StartEncounter name=%s id=%s locked=%d corpse=%d"),
		*GetName(), *EncounterID.ToString(), bEncounterLocked ? 1 : 0, bIsCorpse ? 1 : 0);

	if (bIsCorpse || bEncounterLocked) return;
	bEncounterLocked = true;

	if (!SpawnData || !UnitData)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 데이터 에셋 할당 확인 필요!"), *GetName());
		return;
	}

	// 충돌 즉시 차단
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
		UE_LOG(LogTemp, Error, TEXT("[WorldEnemy] EncounterID is None."));
	}

	const FName WorldLevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	Transfer->SetPendingEncounter(EncounterID, WorldLevelName, bIsBossEncounter);
	Transfer->SetEnemyWorldTransform(GetActorTransform()); // 전투 시작 전 적 위치 저장
	Transfer->SetReturnPoint(WorldLevelName, PlayerActor->GetActorLocation(), PlayerActor->GetActorRotation());

	const FEnemySpawnGroup* Group = nullptr;
	for (const FEnemySpawnGroup& G : SpawnData->SpawnGroups)
	{
		if (G.EncounterID == EncounterID)
		{
			Group = &G;
			break;
		}
	}
	if (!Group && SpawnData->SpawnGroups.Num() > 0)
	{
		Group = &SpawnData->SpawnGroups[0];
		UE_LOG(LogTemp, Warning, TEXT("[WorldEnemy] SpawnGroup fallback to index0."));
	}

	if (Group)
	{
		Transfer->InitEnemyBattleInfo(Group->EnemyList);
		Transfer->SetPendingLootConfig(Group->LootRolls, Group->LootTable, Group->GuaranteedLoot);
	}

	UGameplayStatics::OpenLevel(this, FName("L_Battle"));
}

void AWorldEnemyUnit::ConvertToCorpse()
{
	if (bIsCorpse) return;
	bIsCorpse = true;

	if (DetectSphere)
	{
		DetectSphere->SetGenerateOverlapEvents(false);
		DetectSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetectSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnDetectOverlap);
		DetectSphere->OnComponentEndOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnDetectEndOverlap);
	}

	if (UCapsuleComponent* CapComp = GetCapsuleComponent())
	{
		CapComp->SetGenerateOverlapEvents(false);
		CapComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CapComp->OnComponentBeginOverlap.RemoveDynamic(this, &AWorldEnemyUnit::OnEncounterOverlap);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetHiddenInGame(true);
		MeshComp->SetComponentTickEnabled(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (DeadStaticMeshComp)
	{
		DeadStaticMeshComp->SetHiddenInGame(false);
		DeadStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DeadStaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		DeadStaticMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

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

	// 루팅 프롬프트 이미지 표시
	if (AWorldHUD* WHUD = Cast<AWorldHUD>(PC->GetHUD()))
	{
		WHUD->ShowLootPrompt();
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Press E to Loot"));
	}
}

void AWorldEnemyUnit::OnCorpseEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	DisableInput(PC);
	
	// 루팅 프롬프트 이미지 숨김
	if (AWorldHUD* WHUD = Cast<AWorldHUD>(PC->GetHUD()))
	{
		WHUD->HideLootPrompt();
	}
}

void AWorldEnemyUnit::Loot()
{
	if (!bIsCorpse) return;

	UE_LOG(LogTemp, Warning, TEXT("[Loot] Player looted corpse: %s"), *GetName());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("[Loot] Items acquired (prototype)"));
	}

	Destroy();
}

bool AWorldEnemyUnit::TrySpawnCorpseIfDefeated()
{
	if (EncounterID == NAME_None) return false;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;

	UBattleInfoTransferSubsystem* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	if (!Transfer) return false;

	if (Transfer->IsEncounterLooted(EncounterID))
	{
		Destroy();
		return true;
	}

	TArray<FLootStack> Loot;
	Transfer->TryGetEncounterLoot(EncounterID, Loot);

	if (LootCorpseClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 전투 시작 전 저장된 적 위치 사용, 없으면 현재 위치 fallback
		FTransform SpawnTransform = GetActorTransform();
		if (Transfer->TryGetEnemyWorldTransform(SpawnTransform))
		{
			Transfer->ClearEnemyWorldTransform();
		}

		// NavMesh로 Z 보정 - 항상 바닥에 붙어서 스폰
		if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
		{
			FNavLocation ProjectedLoc;
			if (NavSys->ProjectPointToNavigation(
				SpawnTransform.GetLocation(),
				ProjectedLoc,
				FVector(50.f, 50.f, 500.f)))
			{
				SpawnTransform.SetLocation(ProjectedLoc.Location);
			}
		}

		ALootCorpseActor* Corpse = GetWorld()->SpawnActor<ALootCorpseActor>(
			LootCorpseClass, SpawnTransform, SpawnParams);

		if (Corpse)
		{
			UStaticMesh* CorpseMesh = (UnitData && UnitData->DeadStaticMesh) ? UnitData->DeadStaticMesh : nullptr;
			Corpse->InitCorpse(EncounterID, CorpseMesh, Loot);
		}
	}

	Destroy();
	return true;
}

void AWorldEnemyUnit::PausePatrol()
{
	bPatrolPaused = true;
	// AI 이동 즉시 정지
	if (AAIController* Ctrl = Cast<AAIController>(GetController()))
	{
		Ctrl->StopMovement();
	}
}

void AWorldEnemyUnit::ResumePatrol()
{
	bPatrolPaused = false;
	bWaitingForPatrolMove = false; // 재개 시 이동 명령 허용
	// 현재 상태에 맞게 이동 재개
	if (AIState == EEnemyAIState::Patrol || AIState == EEnemyAIState::Chase)
	{
		MoveToNextPatrolPoint();
	}
}