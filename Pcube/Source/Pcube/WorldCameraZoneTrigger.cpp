#include "WorldCameraZoneTrigger.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "WorldCCTVCameraDirector.h"

AWorldCameraZoneTrigger::AWorldCameraZoneTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void AWorldCameraZoneTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldCameraZoneTrigger::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AWorldCameraZoneTrigger::OnTriggerEndOverlap);

	EnsureDirector();

	if (CameraAnchor.GetLocation().IsNearlyZero() && CameraAnchor.GetRotation().IsIdentity())
	{
		CameraAnchor = GetActorTransform();
	}

	if (bApplyIfPlayerStartsInside)
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			if (TriggerBox && TriggerBox->IsOverlappingActor(PlayerPawn))
			{
				ApplyZoneToPlayer(PlayerPawn);
			}
		}
	}
}

void AWorldCameraZoneTrigger::EnsureDirector()
{
	if (Director)
	{
		return;
	}

	Director = Cast<AWorldCCTVCameraDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldCCTVCameraDirector::StaticClass()));
}

bool AWorldCameraZoneTrigger::IsPlayerActor(AActor* Actor) const
{
	if (!Actor) return false;

	APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn) return false;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	return Pawn == PlayerPawn;
}

void AWorldCameraZoneTrigger::ApplyZoneToPlayer(AActor* PlayerActor)
{
	if (!IsPlayerActor(PlayerActor)) return;

	EnsureDirector();
	if (!Director) return;

	if (bOverrideRotationSpeed)
	{
		Director->SetRotationInterpSpeed(RotationInterpSpeedOverride);
	}

	// 삭제됨: LevelsToLoad / LevelsToUnload / bBlockOnLevelStreaming 관련 코드 전부 제거

	// CCTV Director를 뷰타겟으로 전환
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	// Director 앵커 위치 이동
	if (bUseFadeTransition)
	{
		// 페이드 사용 시 RequestZoneTransition (스트리밍 없는 버전)
		FWorldCameraZoneTransitionRequest Request;
		Request.CameraAnchor       = CameraAnchor;
		Request.AnchorBlendTime    = BlendTime;
		Request.Priority           = Priority;
		Request.bUseFadeTransition = true;
		Request.FadeOutDuration    = FadeOutDuration;
		Request.FadeInDuration     = FadeInDuration;
		Request.bFreezePlayerDuringTransition = bFreezePlayerDuringTransition;
		Request.bBlockOnLevelStreaming = false;

		Director->SetTargetActor(PlayerActor);
		Director->RequestZoneTransition(Request, PlayerActor);
	}
	else
	{
		// 페이드 없이 앵커 이동 후 뷰타겟 전환
		Director->SetTargetActor(PlayerActor);
		Director->RequestAnchorTransform(CameraAnchor, BlendTime, Priority);
	}

	// 플레이어 컨트롤러 뷰타겟을 Director로 전환
	PC->SetViewTargetWithBlend(Director, BlendTime);
}

void AWorldCameraZoneTrigger::RestorePlayerCamera()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return;

	// 뷰타겟을 폰으로 복귀
	PC->SetViewTargetWithBlend(PlayerPawn, BlendTime);
}

void AWorldCameraZoneTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ApplyZoneToPlayer(OtherActor);
}

void AWorldCameraZoneTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bIgnoreEndOverlap)
	{
		return;
	}
}
