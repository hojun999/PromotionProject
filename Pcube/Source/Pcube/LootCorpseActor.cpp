// Fill out your copyright notice in the Description page of Project Settings.


#include "LootCorpseActor.h"
#include "BattleInfoTransferSubsystem.h"
#include "WorldHUD.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

ALootCorpseActor::ALootCorpseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CorpseMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CorpseMeshComp"));
	SetRootComponent(CorpseMeshComp);
	CorpseMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CorpseMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CorpseMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->SetupAttachment(RootComponent);
	InteractBox->SetBoxExtent(FVector(140, 140, 120));
	InteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALootCorpseActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (InteractBox)
	{
		InteractBox->OnComponentBeginOverlap.AddDynamic(this, &ALootCorpseActor::OnInteractBeginOverlap);
		InteractBox->OnComponentEndOverlap.AddDynamic(this, &ALootCorpseActor::OnInteractEndOverlap);
	}
}

void ALootCorpseActor::InitCorpse(FName InEncounterID, UStaticMesh* InCorpseMesh, const TArray<FLootStack>& InLoot)
{
	EncounterID = InEncounterID;
	
	if (CorpseMeshComp && InCorpseMesh)
	{
		CorpseMeshComp->SetStaticMesh(InCorpseMesh);
	}
	
	LootItems = InLoot;
	CompactLoot();
	SaveLootState();
}

void ALootCorpseActor::OnInteractBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	EnableInput(PC);
	
	if (InputComponent && InputComponent != BoundInputComponent)
	{
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ALootCorpseActor::OpenLootUI);
		BoundInputComponent = InputComponent;
	}
	
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(90001, 2.0f, FColor::Green, TEXT("Press E to Loot"));
	}
#endif
}

void ALootCorpseActor::OnInteractEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;
	
	DisableInput(PC);
}

void ALootCorpseActor::OpenLootUI()
{
	if (IsEmpty())
	{
		SaveLootState();
		Destroy();
		return;
	}
	
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) return;
	
	AWorldHUD* WHUD = Cast<AWorldHUD>(PC->GetHUD());
	if (!WHUD) return;
	
	WHUD->ShowLootWindow(this);
}

int32 ALootCorpseActor::RemoveLootCountAt(int32 Index, int32 Count, FLootStack* OutRemoved)
{
	if (!LootItems.IsValidIndex(Index)) return 0;
	if (!LootItems[Index].Item || LootItems[Index].Count <= 0 || Count <= 0) return 0;

	FLootStack& Stack = LootItems[Index];
	const int32 RemovedCount = FMath::Min(Count, Stack.Count);
	if (RemovedCount <= 0) return 0;

	if (OutRemoved)
	{
		OutRemoved->Item = Stack.Item;
		OutRemoved->Count = RemovedCount;
	}

	Stack.Count -= RemovedCount;
	if (Stack.Count <= 0)
	{
		LootItems.RemoveAt(Index);
	}

	CompactLoot();
	SaveLootState();
	return RemovedCount;
}

void ALootCorpseActor::CompactLoot()
{
	for (int32 i = LootItems.Num() - 1; i >= 0; --i)
	{
		if (!LootItems[i].Item || LootItems[i].Count <= 0)
		{
			LootItems.RemoveAt(i);
		}
	}
	
	if (LootItems.Num() > 8)
	{
		LootItems.SetNum(8);
	}
}

void ALootCorpseActor::SaveLootState()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UBattleInfoTransferSubsystem* Transfer = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	if (!Transfer) return;
	
	if (IsEmpty())
	{
		Transfer->MarkEncounterLooted(EncounterID);
	}
	else
	{
		Transfer->SetEncounterLoot(EncounterID, LootItems);
	}
}
