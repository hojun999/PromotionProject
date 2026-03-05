// Fill out your copyright notice in the Description page of Project Settings.


#include "LootCorpseActor.h"
#include "BattleInfoTransferSubsystem.h"
#include "WorldHUD.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

// Sets default values
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

// Called when the game starts or when spawned
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

	// 입력 바인딩을 위해서는 EnableInput이 필요
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
	// 이미 비어있으면 그냥 제거
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
	
	WHUD->ShowLootWindow(this); // WorldHUD가 루팅창 생성&표시 + InputMode/UI 처리
}

bool ALootCorpseActor::TakeLootAt(int32 Index, FLootStack& OutTaken)
{
	if (!LootItems.IsValidIndex(Index)) return false;
	if (!LootItems[Index].Item || LootItems[Index].Count <= 0) return false;
	
	OutTaken = LootItems[Index];
	LootItems.RemoveAt(Index);
	
	CompactLoot();
	SaveLootState();
	return true;
}

void ALootCorpseActor::TakeAllLoot(TArray<FLootStack>& OutTakenAll)
{
	OutTakenAll = LootItems;
	LootItems.Empty();
	
	SaveLootState();
}

void ALootCorpseActor::CompactLoot()
{
	// 빈 / 수량 0 아이템 제거 + 앞으로 당겨서 UI 위치 정렬
	for (int32 i = LootItems.Num() - 1; i >= 0; --i)
	{
		if (!LootItems[i].Item || LootItems[i].Count <= 0)
		{
			LootItems.RemoveAt(i);
		}
	}
	
	// 루트 슬롯 최대 8칸 제한 - 초과는 뒤에서 잘라냄
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
		Transfer->MarkEncounterLooted(EncounterID); // 루팅 완료 => 다음 월드 로드 시 시체 스폰 X
	}
	else
	{
		Transfer->SetEncounterLoot(EncounterID, LootItems); // 잔여 루팅 저장
	}
}
