// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleBaseUnit.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"
#include "EquipmentSubsystem.h"
#include "WeaponDataAsset.h"
#include "WeaponPartDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BattleProjectile.h"
#include "BattleDamageTextActor.h"
#include "Components/WidgetComponent.h"

class APlayerState;

const FName ABattleBaseUnit::Notify_Hit(TEXT("Hit"));
const FName ABattleBaseUnit::Notify_Fire(TEXT("Fire"));
const FName ABattleBaseUnit::Notify_Effect(TEXT("Effect"));

// Sets default values
ABattleBaseUnit::ABattleBaseUnit()
{
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	
	WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComp"));
	WeaponMeshComp->SetupAttachment(RootComponent);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetGenerateOverlapEvents(false);
	WeaponMeshComp->SetVisibility(true, true);
	WeaponMeshComp->SetHiddenInGame(true, true);

	
	// ACharacter를 상속받으므로 깁본 skeletalmesh 숨기기
	// TODO: 이후에 Skeletalmesh 사용할 때 아래 내용 삭제
	// if (GetMesh())
	// {
	// 	GetMesh()->SetHiddenInGame(true);
	// }
	
	// CineCameraArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CineCameraArmComp"));
	// CineCameraArmComp->SetupAttachment(RootComponent);
	// CineCameraArmComp->TargetArmLength = 800.0f;
	// CineCameraArmComp->SetRelativeRotation(FRotator(-30.0f, 45.0f, 0.0f));
	// CineCameraArmComp->bDoCollisionTest = false;
	// CineCameraArmComp->bUsePawnControlRotation = false;
	//
	// CineCameraComp = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCameraComp"));
	// CineCameraComp->SetupAttachment(CineCameraArmComp, USpringArmComponent::SocketName);
	// CineCameraComp->CurrentFocalLength = 50.0f;
	
	CameraArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArmComp"));
	CameraArmComp->SetupAttachment(RootComponent);
	CameraArmComp->TargetArmLength = 800.0f;
	CameraArmComp->SetRelativeRotation(FRotator(-30.0f, 45.0f, 0.0f));
	CameraArmComp->bDoCollisionTest = false;
	CameraArmComp->bUsePawnControlRotation = false;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(CameraArmComp, USpringArmComponent::SocketName);
	
	UIAnchorPoint = CreateDefaultSubobject<USceneComponent>(TEXT("UIAnchorPoint"));
	UIAnchorPoint->SetupAttachment(RootComponent);
	
	DamageTextActorClass = ABattleDamageTextActor::StaticClass();
}

// Called when the game starts or when spawned
void ABattleBaseUnit::BeginPlay()
{
	Super::BeginPlay();

	// 장비 변경 이벤트 바인딩(PartyIndex는 이후에 세팅될 수 있으므로, 이벤트는 미리 걸어둔다)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
		{
			EquipSub->OnEquipmentChanged.RemoveDynamic(this, &ABattleBaseUnit::HandleEquipmentChanged);
			EquipSub->OnEquipmentChanged.AddDynamic(this, &ABattleBaseUnit::HandleEquipmentChanged);
		}
	}

	OnHPChanged.RemoveDynamic(this, &ABattleBaseUnit::HandleEnemyHPBarChanged);
	OnHPChanged.AddDynamic(this, &ABattleBaseUnit::HandleEnemyHPBarChanged);

	RefreshEquipmentVisuals();
	RefreshEnemyHPBar();
}


void ABattleBaseUnit::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
		{
			EquipSub->OnEquipmentChanged.RemoveDynamic(this, &ABattleBaseUnit::HandleEquipmentChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ABattleBaseUnit::RefreshWeaponVisual()
{
	if (!WeaponMeshComp)
	{
		return;
	}

	// 스켈레탈 유닛이 아니면 무기 표시 X
	USkeletalMeshComponent* SkelComp = GetMesh();
	if (!SkelComp || !UnitData)
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
		WeaponMeshComp->SetVisibility(false, true);
		return;
	}

	const FName AttachSocket = UnitData->WeaponAttachSocketName;
	if (AttachSocket == NAME_None)
	{
		// 유닛 데이터에서 소켓을 지정하지 않은 경우 무기 비주얼 숨김
		WeaponMeshComp->SetVisibility(false, true);
		return;
	}

	if (!SkelComp->DoesSocketExist(AttachSocket))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponVisual] %s: Attach socket '%s' not found on SkeletalMesh '%s'"),
	*GetName(), *AttachSocket.ToString(), *GetNameSafe(SkelComp->GetSkeletalMeshAsset()));
		WeaponMeshComp->SetVisibility(false, true);
		return;
	}

	// 1. 우선 장착 시스템에 무기가 있으면 사용
	UWeaponDataAsset* WeaponDA = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEquipmentSubsystem* Equip = GI->GetSubsystem<UEquipmentSubsystem>())
		{
			if (PartyIndex != INDEX_NONE)
			{
				WeaponDA = Equip->GetEquippedWeapon(PartyIndex);
			}
		}
	}

	// 2. fallback: 유닛 기본 무기
	if (!WeaponDA)
	{
		WeaponDA = UnitData->DefaultWeapon;
	}

	if (!WeaponDA || !WeaponDA->WeaponMesh)
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
		WeaponMeshComp->SetVisibility(false, true);
		WeaponMeshComp->SetHiddenInGame(true, true);
		return;
	}

	WeaponMeshComp->SetVisibility(true, true);
	WeaponMeshComp->SetHiddenInGame(false, true);
	WeaponMeshComp->SetStaticMesh(WeaponDA->WeaponMesh);
	WeaponMeshComp->AttachToComponent(
		SkelComp,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		AttachSocket
	);
	
}

void ABattleBaseUnit::HandleEquipmentChanged(int32 ChangedPartyIndex)
{
	if (PartyIndex == INDEX_NONE) return;
	if (ChangedPartyIndex != PartyIndex) return;
	RefreshEquipmentVisuals();
}

void ABattleBaseUnit::HandleEnemyHPBarChanged(float InCurrentHP, float InMaxHP)
{
	RefreshEnemyHPBar();
}

void ABattleBaseUnit::SetEnemyHPBarVisible(bool bVisible)
{
	if (!EnemyHPBarComponent) return;
	EnemyHPBarComponent->SetVisibility(bVisible, true);
	EnemyHPBarComponent->SetHiddenInGame(!bVisible, true);
}

void ABattleBaseUnit::RefreshEnemyHPBar()
{
	if (!EnemyHPBarComponent) return;

	const bool bShouldShow = (PartyIndex == INDEX_NONE) && !IsDead();
	SetEnemyHPBarVisible(bShouldShow);
	if (!bShouldShow) return;

	if (UBattleEnemyHPBarWidget* HPWidget = Cast<UBattleEnemyHPBarWidget>(EnemyHPBarComponent->GetUserWidgetObject()))
	{
		HPWidget->InitForUnitName(UnitData ? UnitData->UnitName : GetName());
		HPWidget->SetHP(CurrentHP, GetMaxHP());
	}
}

void ABattleBaseUnit::RefreshEquipmentVisuals()
{
	if (!WeaponMeshComp)
	{
		return;
	}

	// PartyIndex가 없는 유닛(적 등)은 장비 비주얼을 사용하지 않는다.
	if (PartyIndex == INDEX_NONE)
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
		WeaponMeshComp->SetHiddenInGame(true);
		for (auto& KVP : PartMeshBySocket)
		{
			if (KVP.Value)
			{
				KVP.Value->DestroyComponent();
			}
		}
		PartMeshBySocket.Empty();
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UEquipmentSubsystem* EquipSub = GI ? GI->GetSubsystem<UEquipmentSubsystem>() : nullptr;
	if (!EquipSub)
	{
		return;
	}

	EquipSub->InitializeUnitLoadoutIfMissing(PartyIndex, UnitData);

	// 무기 본체 적용
	UWeaponDataAsset* WeaponDA = EquipSub->GetEquippedWeapon(PartyIndex);
	if (!WeaponDA && UnitData)
	{
		WeaponDA = UnitData->DefaultWeapon;
	}
	if (WeaponDA && WeaponDA->WeaponMesh)
	{
		WeaponMeshComp->SetStaticMesh(WeaponDA->WeaponMesh);
		WeaponMeshComp->SetHiddenInGame(false);
	}
	else
	{
		WeaponMeshComp->SetStaticMesh(nullptr);
		WeaponMeshComp->SetHiddenInGame(true);
	}

	// 무기 본체를 캐릭터 메시 소켓에 붙이고 싶으면 여기서 처리
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		FName AttachSocket = WeaponHoldSocketName;

		if (AttachSocket == NAME_None && UnitData)
		{
			AttachSocket = UnitData->WeaponAttachSocketName;
		}

		if (AttachSocket != NAME_None && Skel->DoesSocketExist(AttachSocket))
		{
			WeaponMeshComp->AttachToComponent(
				Skel,
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				AttachSocket
			);
		}
		else
		{
			WeaponMeshComp->AttachToComponent(
				Skel,
				FAttachmentTransformRules::KeepRelativeTransform
			);
		}
	}

	// 슬롯/파츠 동기화
	TSet<FName> DesiredSockets;
	if (WeaponDA)
	{
		for (const FWeaponModSlotDef& Def : WeaponDA->ModSlots)
		{
			if (Def.SocketName != NAME_None)
			{
				DesiredSockets.Add(Def.SocketName);
			}
		}
	}

	// 무기 슬롯에 없는 파츠 컴포넌트 정리
	for (auto It = PartMeshBySocket.CreateIterator(); It; ++It)
	{
		if (!DesiredSockets.Contains(It.Key()))
		{
			if (It.Value())
			{
				It.Value()->DestroyComponent();
			}
			It.RemoveCurrent();
		}
	}

	for (const FName SocketName : DesiredSockets)
	{
		UWeaponPartDataAsset* PartDA = EquipSub->GetEquippedPart(PartyIndex, SocketName);
		UStaticMeshComponent* PartComp = PartMeshBySocket.FindRef(SocketName);

		// 파츠가 없거나 메시가 없으면 제거
		if (!PartDA || !PartDA->AttachmentMesh || WeaponMeshComp->GetStaticMesh() == nullptr)
		{
			if (PartComp)
			{
				PartComp->DestroyComponent();
				PartMeshBySocket.Remove(SocketName);
			}
			continue;
		}

		// 소켓 검증
		if (!WeaponMeshComp->DoesSocketExist(SocketName))
		{
			UE_LOG(LogTemp, Warning, TEXT("[EquipmentVisual] Weapon mesh socket not found: %s (Weapon=%s Unit=%s)"),
				*SocketName.ToString(), *GetNameSafe(WeaponDA), *GetName());
			continue;
		}

		if (!PartComp)
		{
			PartComp = NewObject<UStaticMeshComponent>(this);
			if (!PartComp) continue;
			PartComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PartComp->SetGenerateOverlapEvents(false);
			PartComp->RegisterComponent();
			PartComp->AttachToComponent(WeaponMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			PartMeshBySocket.Add(SocketName, PartComp);
		}

		PartComp->SetStaticMesh(PartDA->AttachmentMesh);
		PartComp->SetHiddenInGame(false);
	}
}

// BattleContolSubsystem에서 유닛을 spanw시킨 직후 호출
void ABattleBaseUnit::InitUnit(UUnitDataAsset* TransferredUnitData)
{
	if (!TransferredUnitData) return;
	
	// --- 스탯 초기화 ---
	UnitData = TransferredUnitData;
		
	FUnitBaseStats ResolvedStats = UnitData->BaseStats;
	if (PartyIndex != INDEX_NONE)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
			{
				ResolvedStats = EquipSub->ResolveFinalStats(PartyIndex, ResolvedStats);
			}
		}
	}

	const float MaxHP = FMath::Max(1.f, ResolvedStats.MaxHP);
	const float Speed = FMath::Max(1.f, ResolvedStats.Speed);
	const float ATK = FMath::Max(1.f, ResolvedStats.AttackPower);
	
	CurrentHP = MaxHP;
	CurrentSpeed = Speed;
	CurrentAttackPower = ATK;
	bIsDead = false;
		
	UE_LOG(LogTemp, Warning, TEXT("[InitUnit] %s HP=%.1f (MaxHP=%.1f) SPD=%.1f ATK=%.1f UnitData=%s"),
		*GetName(), CurrentHP, UnitData->BaseStats.MaxHP, CurrentSpeed, CurrentAttackPower, *GetNameSafe(UnitData));
	
	
	// --- 스킬포인트  관련 ---
	const int32 MaxSP = GetMaxSkillPoints();
	CurrentSkillPoints = (UnitData && MaxSP > 0)
		? FMath::Clamp(UnitData->BaseSkillPoints, 0, MaxSP)
		: 0;
	
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, MaxSP);
	
	// 아군/스켈레탈 기반 유닛
	if (IsValid(UnitData->BattleSkeletalMesh))
	{
		if (USkeletalMeshComponent* SkelComp = GetMesh())
		{
			SkelComp->SetSkeletalMesh(UnitData->BattleSkeletalMesh);
			
			// 전투용 AnimBP 저용 -> Idle은 AnimBP가 자동으로 재생
			if (UnitData->BattleAnimBlueprintInstance)
			{
				SkelComp->SetAnimationMode((EAnimationMode::AnimationBlueprint)); // AnimBP 구동 모드
				SkelComp->SetAnimInstanceClass(UnitData->BattleAnimBlueprintInstance); // 전투 AnimBP 부착
			}
			
			// UI 초기 갱신용 브로드캐스트
			OnHPChanged.Broadcast(CurrentHP, MaxHP);
			return;
		}
	} // TODO: 적군도 스켈레탈 메쉬로 변경
	else if (IsValid(UnitData->UnitStaticMesh))// 적군의 경우
	{
		if (StaticMeshComp)
		{
			StaticMeshComp->SetStaticMesh((UnitData->UnitStaticMesh));
		
			// 여기서 스케일이나 머티리얼 추가 조정 가능
			return;
		}
	}
	
	
}

void ABattleBaseUnit::FinishAction()
{
	UE_LOG(LogTemp, Log, TEXT("%s 유닛 행동 종료!"), *GetName());
	
	TickBuffDuration_OnActionEnd();
	
	// 턴 종료 후 SP 획득
	if (CurrentSkillData)
	{
		GainSkillPoints(CurrentSkillData->SkillPointGainOnUse);
	}
	
	// 다음 턴에 이전 스킬 잔류로 포인트가 또 들어가는 문제 방지
	CurrentSkillData = nullptr;
	CurrentActionTargets.Empty();
	
	// 신호 발송 - 구독하고 있는 모든 곳에 알림
	if (OnActionFinished.IsBound())
	{
		OnActionFinished.Broadcast();
	}
}

void ABattleBaseUnit::TryFinishAction()
{
	// 아직 몽타주가 안 끝났으면 FinishAction 금지 - 연출 중
	if (!bActionMontageEnded) return;
	
	// 투사체가 남아 있으면 FinishAction 금지 - 충돌/소멸 대기
	if (PendingProjectiles > 0) return;
	
	// 연출 종료 & 모든 투사체 종료된 상태
	FinishAction();
}

void ABattleBaseUnit::BasicAttack(ABattleBaseUnit* Target)
{
	if (!Target || CurrentAttackPower <= 0.f) return;
	
	const float Damage = FMath::Max(1.f, CurrentAttackPower);
	
	UGameplayStatics::ApplyDamage(Target, Damage, GetController(), this, UDamageType::StaticClass());
	
	// ApplyDamage 이후 Target->CurrentHP는 TakeDamage에서 갱신됨(동기 호출)
	const FString Msg = FString::Printf(
		TEXT("[DMG][Basic] %s -> %s : -%.1f  HP %.0f/%.0f"),
		*GetUnitLabel(this),
		*GetUnitLabel(Target),
		Damage,
		Target->CurrentHP,
		Target->GetMaxHP()
	);
	ScreenCombatText(ESkillEffectType::Damage, Msg);

	UE_LOG(LogTemp, Warning, TEXT("[BasicAttack] %s -> %s : %0.1f"),
		*GetName(), *Target->GetName(), Damage);
	
	
}

void ABattleBaseUnit::ExecuteAction(USkillDataAsset* SkillData, AActor* Target)
{
	ABattleBaseUnit* T = Cast<ABattleBaseUnit>(Target);
	
	if (!IsValid(T))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] Invalid single target -> FinishAction"));
		FinishAction();
		return;
	}
	
	TArray<ABattleBaseUnit*> Targets;
	Targets.Add(T);
	
	ExecuteActionOnTargets(SkillData, Targets);
}

void ABattleBaseUnit::ExecuteAction(USkillDataAsset* SkillData, const TArray<AActor*>& Targets)
{
	TArray<ABattleBaseUnit*> UnitTargets;
	UnitTargets.Reserve(Targets.Num());
	
	for (AActor* A : Targets)
	{
		ABattleBaseUnit* U = Cast<ABattleBaseUnit>(A);
		if (IsValid(U) && !U->IsDead())
		{
			UnitTargets.Add(U);
		}
	}
	
	if (UnitTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] Targets empty/invalid -> FinishAction"));
		FinishAction();
		return;
	}
	
	ExecuteActionOnTargets(SkillData, UnitTargets);
}

void ABattleBaseUnit::ExecuteActionOnTargets(USkillDataAsset* SkillData, const TArray<ABattleBaseUnit*>& Targets)
{
	if (!IsValid(SkillData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] SkillData null -> FinishAction"));
		FinishAction();
		return;
	}
	
	// 포인트 소모 체크&처리
	if (!SpendSkillPoints(SkillData->SkillPointCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillPoint] Not enough points. unit=%s cost=%d"),
			*GetName(), SkillData->SkillPointCost);
		FinishAction();
		return;
	}
	
	CurrentSkillData = SkillData;
	
	bActionMontageEnded = false;
	PendingProjectiles = 0;
	CurrentMontageInstanceID = INDEX_NONE;
	
	// 런타임 스킬 스탯 확정 - 타수/효과
	CurrentActionSpec = BuildRuntimeSpec(SkillData); // 액션의 진짜 타수/투사체수/배율 확정
	
	UE_LOG(LogTemp, Warning, TEXT("[Spec] skill=%s effect=%d delivery=%d projClass=%s muzzle=%s projCount=%d hit=%d"),
	*GetNameSafe(SkillData),
	(int32)CurrentActionSpec.EffectType,
	(int32)CurrentActionSpec.DeliveryType,
	*GetNameSafe(CurrentActionSpec.ProjectileClass),
	*CurrentActionSpec.MuzzleSocketName.ToString(),
	CurrentActionSpec.ProjectileCount,
	CurrentActionSpec.HitCount);
	
	ActionTotalHits = CurrentActionSpec.HitCount;
	ActionHitsApplied = 0;
	
	CurrentActionTargets.Empty();
	for (ABattleBaseUnit* T : Targets)
	{
		if (IsValid(T) && !T->IsDead())
		{
			CurrentActionTargets.Add(T);
		}
	}
	
	if (CurrentActionTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] No valid targets after filter -> FinishAction"));
		FinishAction();
		return;
	}
	
	
	// 애니메이션 없으면 딜레이 후 처리 
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage* MontageToPlay = ResolveActionMontage(SkillData);
	if (!AnimInstance || !MontageToPlay)
	{
		const float Delay = GetEnemyNoMontageDelaySeconds(); 
		if (Delay > 0.f) 
		{
			GetWorldTimerManager().ClearTimer(NoMontageActionTimerHandle); 
			GetWorldTimerManager().SetTimer( 
				NoMontageActionTimerHandle, 
				this, 
				&ABattleBaseUnit::HandleNoMontageActionDelayExpired, 
				Delay, 
				false 
			); 
		} 
		else 
		{
			ApplyRemainingHits(); 
			FinishAction(); 
		} 
		return;
	}
	
	// 노티파이 바인딩
	AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	
	// 몽타주 재생 (유닛별 오버라이드 가능)
	const float PlayResult = AnimInstance->Montage_Play(MontageToPlay);
	if (PlayResult <= 0.f)
	{
		ApplyRemainingHits();
		FinishAction();
		return;
	}
	
	// 가장 최근에 재생된 몽타주의 인스턴스 ID를 확보해서 NotifyPayload에서 들어오는 ID와 매칭
	if (FAnimMontageInstance* AMInst = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
	{
		CurrentMontageInstanceID = AMInst->GetInstanceID(); // NotifyPayload.MontageInstanceId와 비교할 값
	}
	
	// 종료 델리게이트
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ABattleBaseUnit::OnActionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
}


void ABattleBaseUnit::ApplyCurrentSkillDamage()
{
	if (!CurrentSkillData || !CurrentActionTarget) return;
	
	const int32 FinalHitCount = FMath::Max(1, CurrentSkillData->BaseHitCount);
	const float DamagePerHit = FMath::Max(1.f, CurrentAttackPower * CurrentSkillData->DamageMultiplier);
	
	// 타겟 배열이 비어있으면 기존 단일 타겟 fallback
	if (CurrentActionTargets.Num() == 0 && CurrentActionTarget)
	{
		CurrentActionTargets.Add(CurrentActionTarget);
	}
	
	for (TWeakObjectPtr<ABattleBaseUnit> TPtr : CurrentActionTargets)
	{
		ABattleBaseUnit* TargetUnit = TPtr.Get();
		if (!IsValid(TargetUnit) || TargetUnit->IsDead()) continue;
		
		for (int32 i = 0; i < FinalHitCount; ++i)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Damage] %s -> %s : %.1f (hit %d/%d)"),
				*GetName(), *TargetUnit->GetName(), DamagePerHit, i + 1, FinalHitCount);

			UGameplayStatics::ApplyDamage(TargetUnit, DamagePerHit, GetController(), this, UDamageType::StaticClass());
		}
	}
}

void ABattleBaseUnit::HandleHitNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("[Notify] %s"), *NotifyName.ToString());
	
	// 다른 몽타주의 Notify가 섞여 들어오면 무시
	if (CurrentMontageInstanceID != INDEX_NONE && Payload.MontageInstanceID != CurrentMontageInstanceID)
	{
		return;
	}
	
	if (NotifyName == Notify_Hit) // Instant 다중 히트 시퀀스 시작
	{
		if (CurrentActionSpec.DeliveryType == ESkillDeliveryType::Instant)
		{
			ApplyInstantHitSequence(); // HitCount만큼 시간차 처리
		}
		else
		{
			ApplyOneHit(); // Projectile은 기존 방식 유지
		}
	}
	else if (NotifyName == Notify_Fire)
	{
		ApplyOneHit(); // Fire 노티파이는 투사체 발사
	}
	else if (NotifyName == Notify_Effect)
	{
		ApplyRemainingHits(); // 한 번에 적용하는 방식으로 확장하고 싶을 때 사용
	}
}

bool ABattleBaseUnit::CanUseSkill(const USkillDataAsset* Skill) const
{
	if (!IsValid(Skill)) return false;
	
	const int32 Max = GetMaxSkillPoints();
	
	if (Max <= 0) return Skill->SkillPointCost <= 0;
	
	return CurrentSkillPoints >= Skill->SkillPointCost;
}

bool ABattleBaseUnit::SpendSkillPoints(int32 Cost)
{
	if (Cost <= 0) return true;
	
	const int32 Max = GetMaxSkillPoints();
	if (Max <= 0) return false;
	
	if (CurrentSkillPoints < Cost) return false;
	
	CurrentSkillPoints = FMath::Clamp(CurrentSkillPoints - Cost, 0, Max);
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, Max);
	return true;
}

void ABattleBaseUnit::GainSkillPoints(int32 Amount)
{
	if (Amount == 0) return;
	
	const int32 Max = GetMaxSkillPoints();
	if (Max <= 0) return;
	
	CurrentSkillPoints = FMath::Clamp(CurrentSkillPoints + Amount, 0, Max);
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, Max);
}

void ABattleBaseUnit::ApplyStun(int32 DurationActions)
{
	if (DurationActions <= 0) return;
	
	// 중첩이 아니라 갱신/연장으로 처리
	StunRemainingActions = FMath::Max(StunRemainingActions, DurationActions);
	
	UE_LOG(LogTemp, Warning, TEXT("[Stun] %s stunned. remainingActions=%d"),
		*GetName(), StunRemainingActions);
}

void ABattleBaseUnit::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	}

	// 노티파이가 부족했으면 남은 웨이브 실행 (투사체 추가 발사 가능)
	ApplyRemainingHits();
	
	bActionMontageEnded = true; // 몽타주 종료 신호
	TryFinishAction(); // 투사체 남아있으면 여기서 안끝남
}

float ABattleBaseUnit::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	const float MaxHP = GetMaxHP();
	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage,  0.f,  MaxHP);

	SpawnDamageText(ActualDamage);

	UE_LOG(LogTemp, Warning, TEXT("%s took %f damage. Remaining HP: %f/%f"), *GetName(), ActualDamage, CurrentHP, MaxHP);

	// UI 갱신
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	if (ActualDamage > 0.f && CurrentHP > 0.f)
	{
		TriggerHitReaction();
	}

	if (CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void ABattleBaseUnit::Heal(float Amount)
{
	if (bIsDead) return;
	if (Amount <= 0.0f) return;
	
	const float MaxHP = GetMaxHP();
	CurrentHP = FMath::Clamp(CurrentHP + Amount,  0.f,  MaxHP);
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}


void ABattleBaseUnit::Die()
{
	if (bIsDead) return;
	bIsDead = true;
	
	UE_LOG(LogTemp, Error, TEXT("BattleBaseUnit: %s has died."), *GetName());
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("[Death] %s died."), *GetName()));
	}
	
	// 전투 중 사망한 유닛에 대한 클릭/타겟팅/충돌 방지
	SetActorEnableCollision(false);
	
	OnUnitDied.Broadcast();
}

void ABattleBaseUnit::NotifyProjectileResolved(ABattleBaseUnit* HitTarget, float Damage, bool bDidHit)
{
	const FString HitMark = bDidHit ? TEXT("HIT") : TEXT("MISS");

	// 화면 출력 (ApplyDamage 수행 여부와 무관하게 결과를 보여줌)
	{
		const float CurHP = IsValid(HitTarget) ? HitTarget->CurrentHP : -1.f;
		const float MaxHP = IsValid(HitTarget) ? HitTarget->GetMaxHP() : -1.f;

		const FString Msg = FString::Printf(
			TEXT("[DMG][Projectile][%s][%s] %s -> %s : -%.1f  HP %.0f/%.0f"),
			*GetSkillLabel(CurrentSkillData),
			*HitMark,
			*GetUnitLabel(this),
			*GetUnitLabel(HitTarget),
			Damage,
			CurHP,
			MaxHP
		);
		ScreenCombatText(ESkillEffectType::Damage, Msg);
	}

	if (bDidHit && IsValid(HitTarget) && !HitTarget->IsDead())
	{
		UGameplayStatics::ApplyDamage(HitTarget, Damage, GetController(), this, UDamageType::StaticClass());

		if (CurrentActionSpec.EffectType == ESkillEffectType::Stun)
		{
			const int32 Dur = FMath::Max(1, CurrentActionSpec.StunDurationActions);
			HitTarget->ApplyStun(Dur);

			const FString Msg = FString::Printf(
				TEXT("[STUN][Projectile][%s] %s -> %s : dur=%d"),
				*GetSkillLabel(CurrentSkillData),
				*GetUnitLabel(this),
				*GetUnitLabel(HitTarget),
				Dur
			);
			ScreenCombatText(ESkillEffectType::Stun, Msg);
		}
	}
	
	PendingProjectiles = FMath::Max(0, PendingProjectiles - 1); // 투사체 1개 처리 완료
	TryFinishAction(); // 마지막 투사체면 여기서 FinishAction까지 진행됨
}

void ABattleBaseUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	
	// 클릭 시 선택 상태 반전 (또는 선택 로직 실행)
	bIsSelected = true;
	
	UE_LOG(LogTemp, Warning, TEXT("%s Unit Selected!"), *GetName());
	
	// UI 띄우는 이벤트 호출 구현부
}

// int32 ABattleBaseUnit::GetSkillNumber()
// {
// 	return UnitData ? UnitData->SkillList.Num() : 0;
// }

UAnimMontage* ABattleBaseUnit::ResolveActionMontage(const USkillDataAsset* Skill) const
{
	if (!Skill) return nullptr;
	
	// 유닛별 오버라이드가 있으면 우선
	if (UnitData)
	{
		if (UAnimMontage* const* Found = UnitData->SkillMontageOverrides.Find(const_cast<USkillDataAsset*>(Skill)))
		{
			return *Found;
		}
	}
	
	// 없으면 스킬 기본 몽타주
	return Skill->ActionMontage;
}

FName ABattleBaseUnit::ResolveMuzzleSocketName(const USkillDataAsset* Skill) const
{
	if (!Skill) return NAME_None;

	if (UnitData)
	{
		if (const FName* Found = UnitData->SkillMuzzleSocketOverrides.Find(const_cast<USkillDataAsset*>(Skill)))
		{
			if (*Found != NAME_None)
			{
				return *Found;
			}
		}
	}

	return Skill->MuzzleSocketName;
}


float ABattleBaseUnit::GetEnemyNoMontageDelaySeconds() const
{
	// 적 유닛은 PartyIndex == INDEX_NONE
	if (PartyIndex != INDEX_NONE) return 0.f;
	
	// UnitData에 값이 있으면 사용, 없으면 기본값(1.5)
	if (UnitData && UnitData->EnemyNoMontageAttackDelaySeconds > 0.f)
	{
		return UnitData->EnemyNoMontageAttackDelaySeconds;
	}
	return 1.5f;
}

void ABattleBaseUnit::HandleNoMontageActionDelayExpired()
{
	// 지연 중 사망 등으로 실행 불가하면 안전 종료
	if (IsDead())
	{
		bActionMontageEnded = true;
		TryFinishAction();
		return;
	}

	ApplyRemainingHits();
	bActionMontageEnded = true;
	TryFinishAction();
}

FSkillRuntimeModifier ABattleBaseUnit::GetSkillRuntimeModifier(const USkillDataAsset* Skill) const
{
	FSkillRuntimeModifier Modifier;
	Modifier.DamageMulMul = 1.f;

	if (PartyIndex == INDEX_NONE)
	{
		return Modifier;
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
		{
			const FWeaponPartEffect Effect = EquipSub->GetTotalEquippedPartEffect(PartyIndex);
			Modifier.BonusHitCount += Effect.BonusHitCount;
			Modifier.BonusProjectileCount += Effect.BonusProjectileCount;
			Modifier.DamageMulAdd += Effect.DamageMulAdd;
			Modifier.DamageMulMul *= Effect.DamageMulMul;
		}
	}

	return Modifier;
}


FSkillRuntimeSpec ABattleBaseUnit::BuildRuntimeSpec(const USkillDataAsset* Skill) const
{
	FSkillRuntimeSpec Spec;
	if (!Skill) return Spec;
	
	Spec.TargetType = Skill->TargetType;
	Spec.EffectType = Skill->EffectType;
	
	Spec.DamageMultiplier = Skill->DamageMultiplier;
	Spec.HitCount = Skill->BaseHitCount;
	
	Spec.HealAmount = Skill->HealAmount;
	Spec.BuffAtk = Skill->BuffAtk;
	Spec.StunDurationActions = Skill->StunDurationActions;
	
	// Delivery(즉발/투사체)
	Spec.DeliveryType = Skill->DeliveryType;
	Spec.ProjectileClass = Skill->ProjectileClass;
	Spec.MuzzleSocketName = ResolveMuzzleSocketName(Skill);
	Spec.ProjectileCount = FMath::Max(1, Skill->BaseProjectileCount);
	Spec.bSplitDamageAcrossProjectiles = Skill->bSplitDamageAcrossProjectiles;
	
	// 효과가 Damage가 아니면 타수는 1로 고정 - 나중에 Heal 다중 확장
	if (Spec.EffectType != ESkillEffectType::Damage)
	{
		Spec.HitCount = 1;
	}
	
	const FSkillRuntimeModifier Modifier = GetSkillRuntimeModifier(Skill);
	Spec.HitCount = FMath::Max(1, Spec.HitCount + Modifier.BonusHitCount);
	Spec.ProjectileCount = FMath::Max(1, Spec.ProjectileCount + Modifier.BonusProjectileCount);
	
	Spec.DamageMultiplier = (Spec.DamageMultiplier + Modifier.DamageMulAdd) * Modifier.DamageMulMul;
	Spec.DamageMultiplier = FMath::Max(0.f, Spec.DamageMultiplier);
	
	return Spec;
}

float ABattleBaseUnit::GetEffectiveAttackPower() const
{
	float AddFlat = 0.f;
	float Mul = 1.f;
	
	for (const FActiveAtkBuff& B : ActiveAtkBuffs)
	{
		AddFlat += B.AddFlat;
		Mul *= (1.f + B.AddMultiplier);
	}
	
	return FMath::Max(0.f, (CurrentAttackPower + AddFlat) * Mul);
}

float ABattleBaseUnit::GetMaxHP() const
{
	if (!UnitData)
	{
		return 100.f;
	}

	FUnitBaseStats Resolved = UnitData->BaseStats;
	if (PartyIndex != INDEX_NONE)
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (const UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
			{
				Resolved = EquipSub->ResolveFinalStats(PartyIndex, Resolved);
			}
		}
	}

	return FMath::Max(1.f, Resolved.MaxHP);
}


void ABattleBaseUnit::RefreshBaseCombatStatsFromEquipment(bool bResetCurrentHPToMax)
{
	if (!UnitData)
	{
		return;
	}

	FUnitBaseStats Resolved = UnitData->BaseStats;
	if (PartyIndex != INDEX_NONE)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UEquipmentSubsystem* EquipSub = GI->GetSubsystem<UEquipmentSubsystem>())
			{
				EquipSub->InitializeUnitLoadoutIfMissing(PartyIndex, UnitData);
				Resolved = EquipSub->ResolveFinalStats(PartyIndex, Resolved);
			}
		}
	}

	const float OldMaxHP = UnitData ? FMath::Max(1.f, UnitData->BaseStats.MaxHP) : 100.f;
	const float NewMaxHP = FMath::Max(1.f, Resolved.MaxHP);
	CurrentSpeed = Resolved.Speed;
	CurrentAttackPower = Resolved.AttackPower;

	if (bResetCurrentHPToMax)
	{
		CurrentHP = NewMaxHP;
	}
	else
	{
		const float Ratio = (OldMaxHP > KINDA_SMALL_NUMBER) ? FMath::Clamp(CurrentHP / OldMaxHP, 0.f, 1.f) : 1.f;
		CurrentHP = FMath::Clamp(NewMaxHP * Ratio, 0.f, NewMaxHP);
	}

	OnHPChanged.Broadcast(CurrentHP, NewMaxHP);
}

void ABattleBaseUnit::ApplyHealToUnit(ABattleBaseUnit* Target, float HealAmount)
{
	if (!IsValid(Target) || Target->IsDead()) return;
	const float MaxHP = Target->GetMaxHP();
	Target->CurrentHP = FMath::Clamp(Target->CurrentHP + HealAmount, 0.f, MaxHP);

	Target->OnHPChanged.Broadcast(Target->CurrentHP, MaxHP); // 힐 이후 UI 갱신
	
	UE_LOG(LogTemp, Warning, TEXT("[Heal] %s +%.1f => %.1f/%.1f"),
		*Target->GetName(), HealAmount, Target->CurrentHP, MaxHP);
}

// Instant 타입 다중 히트를 시간차를 두고 순차 적용
void ABattleBaseUnit::ApplyInstantHitSequence()
{ 
	if (!CurrentSkillData) return;
	
	GetWorldTimerManager().ClearTimer(HitSequenceTimerHandle);
	
	const float Interval = FMath::Max(0.f, CurrentSkillData->HitSpawnInterval);
	
	if (FMath::IsNearlyZero(Interval) || ActionTotalHits <= 1)
	{
		// 인터벌 0이거나 1타면 즉시 전부 적용
		ApplyRemainingHits();
		return;
	}
	
	// 첫 타는 즉시, 이후는 Interval 간격으로 타이머 반복
	ApplyOneHit();
	
	if (ActionHitsApplied < ActionTotalHits) 
	{ 
		GetWorldTimerManager().SetTimer( 
			HitSequenceTimerHandle, 
			this, 
			&ABattleBaseUnit::OnHitSequenceTimerTick, 
			Interval, 
			true // 반복 
		); 
	} 
}

// HitSequence 타이머 틱 - 한 번 호출될 때마다 ApplyOneHit 1회 
void ABattleBaseUnit::OnHitSequenceTimerTick() 
{ 
	if (ActionHitsApplied >= ActionTotalHits) 
	{ 
		GetWorldTimerManager().ClearTimer(HitSequenceTimerHandle); 
		return; 
	} 
	
	ApplyOneHit(); 
	
	if (ActionHitsApplied >= ActionTotalHits) 
	{ 
		GetWorldTimerManager().ClearTimer(HitSequenceTimerHandle); 
	} 
} 

// 1hit - 데미지/힐/버프 모두 여기서 처리
void ABattleBaseUnit::ApplyOneHit()
{
	if (!CurrentSkillData) return;
	
	const FSkillRuntimeSpec& Spec = CurrentActionSpec; // 캐싱된 스펙 사용하여 일관성 보장
	
	// 이미 타수 다 쳤으면 무시
	if (ActionHitsApplied >= ActionTotalHits) return;
	
	// 유효 타겟만 정리
	TArray<ABattleBaseUnit*> Targets;
	for (auto& W : CurrentActionTargets)
	{
		if (ABattleBaseUnit* T = W.Get())
		{
			if (!T->IsDead())
			{
				Targets.Add(T);
			}
		}
	}
	if (Targets.Num() == 0)
	{
		// 남은 히트가 있어도 적용할 대상이 사라진 경우(첫 타격에 사망 등)
		// ApplyRemainingHits() 무한 루프 방지를 위해 강제로 완료 처리
		ActionHitsApplied = ActionTotalHits;
		return;
	}

	switch (Spec.EffectType)
	{
	case ESkillEffectType::Damage:
		{
			const float BaseATK = GetEffectiveAttackPower();
			const float Damage = FMath::Max(1.f, BaseATK * Spec.DamageMultiplier);
				
			if (Spec.DeliveryType == ESkillDeliveryType::Projectile)
			{
				SpawnProjectileVolley(Targets, Damage); // 발사
			}
			else
			{
				for (ABattleBaseUnit* T : Targets)
				{
					UGameplayStatics::ApplyDamage(T, Damage, GetController(), this, UDamageType::StaticClass());
					
					const FString Msg = FString::Printf(
			TEXT("[DMG][Instant][%s] %s -> %s : -%.1f  HP %.0f/%.0f  (%d/%d)"),
					*GetSkillLabel(CurrentSkillData),
					*GetUnitLabel(this),
					*GetUnitLabel(T),
					Damage,
					T->CurrentHP,
					T->GetMaxHP(),
					ActionHitsApplied + 1,
					ActionTotalHits
				);
					ScreenCombatText(ESkillEffectType::Damage, Msg);
				}
			}
					
			ActionHitsApplied++;
			break;
		}
		
	case ESkillEffectType::Heal:
		{
			for (ABattleBaseUnit* T : Targets)
			{
				ApplyHealToUnit(T, Spec.HealAmount);
				
				const FString Msg = FString::Printf(
			TEXT("[HEAL][%s] %s -> %s : +%.1f  HP %.0f/%.0f"),
					*GetSkillLabel(CurrentSkillData),
					*GetUnitLabel(this),
					*GetUnitLabel(T),
					Spec.HealAmount,
					T->CurrentHP,
					T->GetMaxHP()
				);
				ScreenCombatText(ESkillEffectType::Heal, Msg);
			}
			
			// Heal은 1회로 끝
			ActionHitsApplied = ActionTotalHits;
			break;
		}
		
	case ESkillEffectType::BuffATK:
		{
			for (ABattleBaseUnit* T : Targets)
			{
				FActiveAtkBuff Buff;
				Buff.AddMultiplier = Spec.BuffAtk.AddMultiplier;
				Buff.AddFlat = Spec.BuffAtk.AddFlat;
				Buff.RemainingActions = FMath::Max(1, Spec.BuffAtk.DurationActions);

				T->ActiveAtkBuffs.Add(Buff);

				const FString Msg = FString::Printf(
					TEXT("[BUFF ATK][%s] %s -> %s : +flat=%.1f +mul=%.2f dur=%d"),
					*GetSkillLabel(CurrentSkillData),
					*GetUnitLabel(this),
					*GetUnitLabel(T),
					Buff.AddFlat,
					Buff.AddMultiplier,
					Buff.RemainingActions
				);
				ScreenCombatText(ESkillEffectType::BuffATK, Msg);
			}

			ActionHitsApplied = ActionTotalHits;
			break;
		}
		
	case ESkillEffectType::Stun:
		{
			for (ABattleBaseUnit* Unit : Targets)
			{
				const int32 Dur = FMath::Max(1, Spec.StunDurationActions);
				Unit->ApplyStun(FMath::Max(1, Spec.StunDurationActions));
				
				const FString Msg = FString::Printf(
			TEXT("[STUN][%s] %s -> %s : dur=%d"),
					*GetSkillLabel(CurrentSkillData),
					*GetUnitLabel(this),
					*GetUnitLabel(Unit),
					Dur
				);
				ScreenCombatText(ESkillEffectType::Stun, Msg);
			}
			ActionHitsApplied = ActionTotalHits;
			break;
		}
	}
}

void ABattleBaseUnit::ApplyRemainingHits()
{
	// 안전장치: 타겟 소멸/무효화 등으로 ApplyOneHit이 진척 없이 return하면 무한 루프가 발생할 수 있음
	int32 Guard = 0;
	while (ActionHitsApplied < ActionTotalHits)
	{
		const int32 Before = ActionHitsApplied;
		ApplyOneHit();
		
		// 진척이 없으면 강제 완료 처리
		if (ActionHitsApplied == Before)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Action] ApplyRemainingHits made no progress. Forcing completion. unit=%s skill=%s"),
				*GetName(), *GetSkillLabel(CurrentSkillData));
			ActionHitsApplied = ActionTotalHits;
			break;
		}
		
		// 추가 안전장치 (예상치 못한 상태에서 무한 루프 방지)
		if (++Guard > 64)
		{
			UE_LOG(LogTemp, Error, TEXT("[Action] ApplyRemainingHits guard overflow. unit=%s skill=%s"),
				*GetName(), *GetSkillLabel(CurrentSkillData));
			ActionHitsApplied = ActionTotalHits;
			break;
		}
	}
}

void ABattleBaseUnit::TickBuffDuration_OnActionEnd()
{
	// 버프 지속 시간 처리 - 감소
	for (int32 i = ActiveAtkBuffs.Num() - 1; i >= 0; i--)
	{
		ActiveAtkBuffs[i].RemainingActions--;
		if (ActiveAtkBuffs[i].RemainingActions <= 0)
		{
			ActiveAtkBuffs.RemoveAt(i);
		}
	}
	
	// 스턴 지속 시간 처리 - 본인 턴 종료 기준으로 1 감소
	if (StunRemainingActions > 0)
	{
		StunRemainingActions--;
	}
}

void ABattleBaseUnit::SpawnProjectileVolley(const TArray<ABattleBaseUnit*>& Targets, float WaveDamage)
{
	const FSkillRuntimeSpec& Spec = CurrentActionSpec;
	
	if (!Spec.ProjectileClass)
	{
		// 투사체 클래스 없으면 안전하게 즉발로 폴백
		for (ABattleBaseUnit* T : Targets)
		{
			UGameplayStatics::ApplyDamage(T, WaveDamage, GetController(), this, UDamageType::StaticClass());
		}
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	const int32 ProjCount = FMath::Max(1, Spec.ProjectileCount);
	const float DamagePerProjectile = Spec.bSplitDamageAcrossProjectiles
		? (WaveDamage / (float)ProjCount) // 투사체 수가 늘어도 총합 유지
		: WaveDamage;					// 투사체 수가 늘면 총합 증가

	// "겹치지 않게" + "순차 발사" 설정 (SkillDataAsset에서 제어)
	const float Interval = (CurrentSkillData)
		? FMath::Max(0.f, CurrentSkillData->ProjectileSpawnInterval)
		: 0.f;

	for (ABattleBaseUnit* Target : Targets)
	{
		if (!IsValid(Target) || Target->IsDead()) continue;

		for (int32 i = 0; i < ProjCount; ++i)
		{
			// 스폰이 실패하거나, 타겟이 중간에 죽어서 스폰하지 않아도 "대기 카운트"는 정리되어야 함
			PendingProjectiles++;

			const float Delay = Interval * (float)i;
			if (Delay <= 0.f)
			{
				SpawnSingleProjectile(TWeakObjectPtr<ABattleBaseUnit>(Target), DamagePerProjectile, i, ProjCount);
			}
			else
			{
				FTimerDelegate D = FTimerDelegate::CreateUObject(
					this,
					&ABattleBaseUnit::SpawnSingleProjectile,
					TWeakObjectPtr<ABattleBaseUnit>(Target),
					DamagePerProjectile,
					i,
					ProjCount
				);
				FTimerHandle TempHandle;
				World->GetTimerManager().SetTimer(TempHandle, D, Delay, false);
			}
		}
	}
}

void ABattleBaseUnit::SpawnSingleProjectile(TWeakObjectPtr<ABattleBaseUnit> Target, float DamagePerProjectile, int32 ProjectileIndex, int32 ProjectileCount)
{
	ABattleBaseUnit* TargetUnit = Target.Get();
	if (!IsValid(TargetUnit) || TargetUnit->IsDead())
	{
		PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
		TryFinishAction();
		return;
	}

	const FSkillRuntimeSpec& Spec = CurrentActionSpec;
	UWorld* World = GetWorld();
	if (!World || !Spec.ProjectileClass)
	{
		PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
		TryFinishAction();
		return;
	}

	// 발사 위치(유닛/스킬이 지정한 소켓 + 로컬 오프셋)
	FTransform MuzzleTM = GetActorTransform();
	if (GetMesh() && Spec.MuzzleSocketName != NAME_None && GetMesh()->DoesSocketExist(Spec.MuzzleSocketName))
	{
		MuzzleTM = GetMesh()->GetSocketTransform(Spec.MuzzleSocketName, RTS_World);
	}

	FVector SpawnLoc = MuzzleTM.GetLocation();
	if (CurrentSkillData)
	{
		SpawnLoc = MuzzleTM.TransformPosition(CurrentSkillData->ProjectileSpawnLocalOffset);
	}

	// 다중 투사체: 좌우로 벌려서 겹침 방지
	const float Spacing = (CurrentSkillData) ? CurrentSkillData->ProjectileLateralSpacing : 0.f;
	if (ProjectileCount > 1 && !FMath::IsNearlyZero(Spacing))
	{
		const float Centered = (float)ProjectileIndex - ((float)ProjectileCount - 1.f) * 0.5f;
		const FVector Right = MuzzleTM.GetRotation().GetRightVector();
		SpawnLoc += Right * (Centered * Spacing);
	}

	const FRotator SpawnRot = (TargetUnit->GetActorLocation() - SpawnLoc).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABattleProjectile* P = World->SpawnActor<ABattleProjectile>(Spec.ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
	if (!P)
	{
		PendingProjectiles = FMath::Max(0, PendingProjectiles - 1);
		TryFinishAction();
		return;
	}

	P->InitProjectile(this, TargetUnit, DamagePerProjectile);
}

FColor ABattleBaseUnit::GetEffectColor(ESkillEffectType EffectType)
{
	switch (EffectType)
	{
	case ESkillEffectType::Damage:  return FColor::Red;
	case ESkillEffectType::Heal:    return FColor::Green;
	case ESkillEffectType::BuffATK: return FColor::Cyan;
	case ESkillEffectType::Stun:    return FColor(255, 165, 0); // Orange
	default:                        return FColor::White;
	}
}

FString ABattleBaseUnit::GetUnitLabel(const ABattleBaseUnit* Unit)
{
	if (!IsValid(Unit)) return TEXT("None");
	if (Unit->UnitData && !Unit->UnitData->UnitName.IsEmpty())
	{
		return Unit->UnitData->UnitName; // 사람이 읽기 쉬운 이름 우선
	}
	return Unit->GetName();
}

FString ABattleBaseUnit::GetSkillLabel(const USkillDataAsset* Skill)
{
	if (IsValid(Skill) && !Skill->SkillName.IsEmpty())
	{
		return Skill->SkillName;
	}
	return TEXT("BasicAttack");
}

void ABattleBaseUnit::SpawnDamageText(float DamageAmount)
{
	if (DamageAmount <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("들어온 대미지 0이하"));
		return;
	} 
	if (!DamageTextActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("대미지 텍스트 액터 클래스 존재 X"));
		return;
	} 

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, 150.f);
	if (UIAnchorPoint)
	{
		SpawnLoc = UIAnchorPoint->GetComponentLocation();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABattleDamageTextActor* TextActor = World->SpawnActor<ABattleDamageTextActor>(DamageTextActorClass, SpawnLoc, FRotator(0, 45.0f, 0), Params);
	if (TextActor)
	{
		TextActor->InitText(DamageAmount, false);
	}
	
}

void ABattleBaseUnit::TriggerHitReaction()
{
	// 적(PartyIndex == INDEX_NONE): 파티클
	// 아군(PartyIndex != INDEX_NONE): 피격 몽타주
	if (!UnitData) return;

	if (PartyIndex != INDEX_NONE)
	{
		// 아군 피격 몽타주
		if (UnitData->HitReactMontage && GetMesh())
		{
			if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
			{
				// 너무 잦은 타격에서 계속 재시작되는 것을 방지
				if (!Anim->Montage_IsPlaying(UnitData->HitReactMontage))
				{
					Anim->Montage_Play(UnitData->HitReactMontage, 1.f);
				}
			}
		}
		return;
	}

	// 적 피격 파티클
	if (!UnitData->EnemyHitParticle) return;

	USceneComponent* AttachComp = nullptr;
	if (GetMesh() && GetMesh()->GetSkeletalMeshAsset())
	{
		AttachComp = GetMesh();
	}
	else if (StaticMeshComp)
	{
		AttachComp = StaticMeshComp;
	}

	if (!AttachComp)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), UnitData->EnemyHitParticle, GetActorLocation() + UnitData->EnemyHitParticleOffset);
		return;
	}

	// 소켓 있으면 소켓, 없으면 위치+오프셋
	if (USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(AttachComp))
	{
		if (UnitData->EnemyHitParticleSocket != NAME_None && Skel->DoesSocketExist(UnitData->EnemyHitParticleSocket))
		{
			UGameplayStatics::SpawnEmitterAttached(
				UnitData->EnemyHitParticle,
				Skel,
				UnitData->EnemyHitParticleSocket,
				UnitData->EnemyHitParticleOffset,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true
			);
			return;
		}
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), UnitData->EnemyHitParticle, AttachComp->GetComponentLocation() + UnitData->EnemyHitParticleOffset);
}

void ABattleBaseUnit::ScreenCombatText(ESkillEffectType EffectType, const FString& Text, float Duration) const
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			Duration,
			GetEffectColor(EffectType),
			Text
		);
	}
#endif
}