#include "WorldCameraZoneTrigger.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
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
	if (!IsPlayerActor(PlayerActor))
	{
		return;
	}

	EnsureDirector();
	if (!Director)
	{
		return;
	}

	if (bOverrideRotationSpeed)
	{
		Director->SetRotationInterpSpeed(RotationInterpSpeedOverride);
	}

	FWorldCameraZoneTransitionRequest Request;
	Request.CameraAnchor = CameraAnchor;
	Request.AnchorBlendTime = BlendTime;
	Request.Priority = Priority;
	Request.LevelsToLoad = LevelsToLoad;
	Request.LevelsToUnload = LevelsToUnload;
	Request.bUseFadeTransition = bUseFadeTransition;
	Request.FadeOutDuration = FadeOutDuration;
	Request.FadeInDuration = FadeInDuration;
	Request.bFreezePlayerDuringTransition = bFreezePlayerDuringTransition;
	Request.bBlockOnLevelStreaming = bBlockOnLevelStreaming;

	Director->RequestZoneTransition(Request, PlayerActor);
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

	// Current design keeps the active camera/streamed view until another zone takes over.
}
