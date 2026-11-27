// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "basic_tps/Core/Presentation/TargetOutlineComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/HeroDataComponent.h"
#include "basic_tps/Core/NotifyState/CombatGameplayTags.h"
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "Engine/DataTable.h"


// Sets default values
AHeroCharacter::AHeroCharacter()
{
	TargetOutlineComp = CreateDefaultSubobject<UTargetOutlineComponent>(TEXT("TargetOutline"));
	TargetOutlineComp->bShowOutline = false;
	TargetOutlineComp->AddTickPrerequisiteComponent(TargetingComp);

	StaminaZoneCapacities = {
		HeroStaminaDefaults::DefaultExhaustedCapacity,
		HeroStaminaDefaults::DefaultFatiguedCapacity,
		HeroStaminaDefaults::DefaultNormalCapacity,
		HeroStaminaDefaults::DefaultVigorousCapacity
	};
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	SyncStaminaZoneConfig();
	LastBroadcastZone = GetCurrentStaminaZone();
	LastAttackZoneSnapshot = LastBroadcastZone;

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
		{
			AnimInst->OnMontageStarted.AddUniqueDynamic(this, &AHeroCharacter::HandleMontageStarted);
			AnimInst->OnMontageEnded.AddUniqueDynamic(this, &AHeroCharacter::HandleMontageEnded);
		}
	}
}

void AHeroCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
		{
			AnimInst->OnMontageStarted.RemoveDynamic(this, &AHeroCharacter::HandleMontageStarted);
			AnimInst->OnMontageEnded.RemoveDynamic(this, &AHeroCharacter::HandleMontageEnded);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AHeroCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	CharacterDataComp->Faction = ECombatFaction::Hero;
	HeroDataComp = NewObject<UHeroDataComponent>(this);
	HeroDataComp->RegisterComponent();
}

void AHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SyncStaminaZoneConfig();
}

#if WITH_EDITOR
void AHeroCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureStaminaArrayNormalized();
	SyncStaminaZoneConfig();
	SetCurrentRage(CurrentRage);
}
#endif

bool AHeroCharacter::CanJumpInternal_Implementation() const
{
	if (!CanHeroJump())
	{
		return false;
	}
	return Super::CanJumpInternal_Implementation();
}

void AHeroCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (JumpStaminaCost > 0)
	{
		ConsumeStamina(JumpStaminaCost, false);
	}
}

void AHeroCharacter::EnsureStaminaArrayNormalized()
{
	static const int32 DefaultValues[HeroStaminaDefaults::ZoneCount] = {
		HeroStaminaDefaults::DefaultExhaustedCapacity,
		HeroStaminaDefaults::DefaultFatiguedCapacity,
		HeroStaminaDefaults::DefaultNormalCapacity,
		HeroStaminaDefaults::DefaultVigorousCapacity
	};

	if (StaminaZoneCapacities.Num() < HeroStaminaDefaults::ZoneCount)
	{
		const int32 OldNum = StaminaZoneCapacities.Num();
		StaminaZoneCapacities.SetNum(HeroStaminaDefaults::ZoneCount);
		for (int32 i = OldNum; i < HeroStaminaDefaults::ZoneCount; ++i)
		{
			StaminaZoneCapacities[i] = DefaultValues[i];
		}
	}
	else if (StaminaZoneCapacities.Num() > HeroStaminaDefaults::ZoneCount)
	{
		StaminaZoneCapacities.SetNum(HeroStaminaDefaults::ZoneCount);
	}

	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		StaminaZoneCapacities[i] = FMath::Max(0, StaminaZoneCapacities[i]);
	}
}

void AHeroCharacter::SyncStaminaZoneConfig()
{
	EnsureStaminaArrayNormalized();

	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		CachedZoneCapacities[i] = StaminaZoneCapacities[i];
	}

	if (IsValid(CharacterDataComp))
	{
		const int32 TotalMax = GetTotalStaminaMax();
		CharacterDataComp->Attributes[AttributeEnum::MP] = TotalMax;
		if (CharacterDataComp->GetCurrentMP() > TotalMax)
		{
			CharacterDataComp->SetCurrentMP(TotalMax);
			StaminaRegenAccumulator = 0.0f;
		}
	}

	CheckAndBroadcastZoneChange();
}

TArray<int32> AHeroCharacter::GetNormalizedStaminaZoneCapacities() const
{
	static const int32 DefaultValues[HeroStaminaDefaults::ZoneCount] = {
		HeroStaminaDefaults::DefaultExhaustedCapacity,
		HeroStaminaDefaults::DefaultFatiguedCapacity,
		HeroStaminaDefaults::DefaultNormalCapacity,
		HeroStaminaDefaults::DefaultVigorousCapacity
	};

	TArray<int32> Result;
	Result.SetNum(HeroStaminaDefaults::ZoneCount);
	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		Result[i] = StaminaZoneCapacities.IsValidIndex(i) ? FMath::Max(0, StaminaZoneCapacities[i]) : DefaultValues[i];
	}
	return Result;
}

int32 AHeroCharacter::GetStaminaZoneCapacity(EStaminaZone Zone) const
{
	const int32 Index = static_cast<int32>(Zone);
	if (Index < 0 || Index >= HeroStaminaDefaults::ZoneCount)
	{
		return 0;
	}
	const TArray<int32> Normalized = GetNormalizedStaminaZoneCapacities();
	return Normalized[Index];
}

void AHeroCharacter::SetStaminaZoneCapacity(EStaminaZone Zone, int32 NewCapacity)
{
	const int32 Index = static_cast<int32>(Zone);
	if (Index < 0 || Index >= HeroStaminaDefaults::ZoneCount)
	{
		return;
	}
	EnsureStaminaArrayNormalized();
	StaminaZoneCapacities[Index] = FMath::Max(0, NewCapacity);
	SyncStaminaZoneConfig();
}

void AHeroCharacter::SetStaminaZoneCapacities(const TArray<int32>& NewCapacities)
{
	StaminaZoneCapacities = NewCapacities;
	SyncStaminaZoneConfig();
}

int32 AHeroCharacter::GetNormalStaminaCap() const
{
	const TArray<int32> Z = GetNormalizedStaminaZoneCapacities();
	return Z[static_cast<int32>(EStaminaZone::Exhausted)]
		+ Z[static_cast<int32>(EStaminaZone::Fatigued)]
		+ Z[static_cast<int32>(EStaminaZone::Normal)];
}

int32 AHeroCharacter::GetTotalStaminaMax() const
{
	const TArray<int32> Z = GetNormalizedStaminaZoneCapacities();
	return Z[static_cast<int32>(EStaminaZone::Exhausted)]
		+ Z[static_cast<int32>(EStaminaZone::Fatigued)]
		+ Z[static_cast<int32>(EStaminaZone::Normal)]
		+ Z[static_cast<int32>(EStaminaZone::Vigorous)];
}

void AHeroCharacter::GetStaminaZoneBounds(EStaminaZone Zone, int32& OutStart, int32& OutEnd) const
{
	const TArray<int32> Z = GetNormalizedStaminaZoneCapacities();
	const int32 TargetIdx = FMath::Clamp(static_cast<int32>(Zone), 0, HeroStaminaDefaults::ZoneCount - 1);

	int32 Running = 0;
	for (int32 i = 0; i < TargetIdx; ++i)
	{
		Running += Z[i];
	}
	OutStart = Running;
	OutEnd = Running + Z[TargetIdx];
}

EStaminaZone AHeroCharacter::GetStaminaZoneForValue(float StaminaValue) const
{
	const TArray<int32> Z = GetNormalizedStaminaZoneCapacities();
	const int32 ExhaustedCap = Z[static_cast<int32>(EStaminaZone::Exhausted)];
	const int32 FatiguedCap  = Z[static_cast<int32>(EStaminaZone::Fatigued)];
	const int32 NormalCap    = Z[static_cast<int32>(EStaminaZone::Normal)];
	const int32 VigorousCap  = Z[static_cast<int32>(EStaminaZone::Vigorous)];

	const float ExhaustedEnd = static_cast<float>(ExhaustedCap);
	const float FatiguedEnd  = static_cast<float>(ExhaustedCap + FatiguedCap);
	const float NormalEnd    = static_cast<float>(ExhaustedCap + FatiguedCap + NormalCap);

	// 严格大于普通上限才进入活力区，保证自然恢复至普通上限时留在普通区
	if (VigorousCap > 0 && StaminaValue > NormalEnd)
	{
		return EStaminaZone::Vigorous;
	}
	if (NormalCap > 0 && StaminaValue > FatiguedEnd)
	{
		return EStaminaZone::Normal;
	}
	if (FatiguedCap > 0 && StaminaValue > ExhaustedEnd)
	{
		return EStaminaZone::Fatigued;
	}
	if (ExhaustedCap > 0)
	{
		return EStaminaZone::Exhausted;
	}
	if (FatiguedCap > 0)
	{
		return EStaminaZone::Fatigued;
	}
	if (NormalCap > 0)
	{
		return EStaminaZone::Normal;
	}
	return EStaminaZone::Vigorous;
}

EStaminaZone AHeroCharacter::GetCurrentStaminaZone() const
{
	const float CurrentStamina = IsValid(CharacterDataComp) ? static_cast<float>(CharacterDataComp->GetCurrentMP()) : 0.0f;
	return GetStaminaZoneForValue(CurrentStamina);
}

float AHeroCharacter::GetCurrentStaminaFloat() const
{
	if (!IsValid(CharacterDataComp))
	{
		return 0.0f;
	}
	const float BaseMP = static_cast<float>(CharacterDataComp->GetCurrentMP());
	const float NormalCap = static_cast<float>(GetNormalStaminaCap());
	if (BaseMP < NormalCap && StaminaRegenAccumulator > 0.0f)
	{
		return FMath::Min(BaseMP + StaminaRegenAccumulator, NormalCap);
	}
	return BaseMP;
}

float AHeroCharacter::GetStaminaZoneFillPercent(EStaminaZone Zone) const
{
	int32 Start = 0;
	int32 End = 0;
	GetStaminaZoneBounds(Zone, Start, End);
	const int32 Cap = End - Start;
	if (Cap <= 0)
	{
		return 0.0f;
	}
	const float Stamina = GetCurrentStaminaFloat();
	return FMath::Clamp((Stamina - static_cast<float>(Start)) / static_cast<float>(Cap), 0.0f, 1.0f);
}

float AHeroCharacter::GetStaminaZoneWidthRatio(EStaminaZone Zone) const
{
	const int32 Total = GetTotalStaminaMax();
	if (Total <= 0)
	{
		return 0.25f;
	}
	return static_cast<float>(GetStaminaZoneCapacity(Zone)) / static_cast<float>(Total);
}

bool AHeroCharacter::CanHeroAttack() const
{
	return IsAlive() && GetCurrentStaminaZone() != EStaminaZone::Exhausted;
}

bool AHeroCharacter::CanHeroJump() const
{
	return IsAlive() && GetCurrentStaminaZone() != EStaminaZone::Exhausted;
}

bool AHeroCharacter::CanHeroDodge() const
{
	return IsAlive() && GetCurrentStaminaZone() != EStaminaZone::Exhausted;
}

float AHeroCharacter::GetAttackDamageMultiplierForZone(EStaminaZone Zone) const
{
	switch (Zone)
	{
	case EStaminaZone::Exhausted:
		// 正常情况下无力区无法起手攻击；若外部直接触发，按疲劳倍率保底
		return FatiguedAttackDamageMultiplier;
	case EStaminaZone::Fatigued:
		return FatiguedAttackDamageMultiplier;
	case EStaminaZone::Normal:
		return NormalAttackDamageMultiplier;
	case EStaminaZone::Vigorous:
		return VigorousAttackDamageMultiplier;
	default:
		return NormalAttackDamageMultiplier;
	}
}

float AHeroCharacter::GetCurrentAttackDamageMultiplier() const
{
	// 若 3 秒内有攻击起手快照，优先按出手瞬间的体力区间结算（防止普攻耗尽体力跌入下区后伤害误降）
	if (const UWorld* World = GetWorld())
	{
		if ((World->GetTimeSeconds() - LastAttackSnapshotTime) <= 3.0)
		{
			return GetAttackDamageMultiplierForZone(LastAttackZoneSnapshot);
		}
	}
	return GetAttackDamageMultiplierForZone(GetCurrentStaminaZone());
}

void AHeroCharacter::CaptureAttackStaminaZoneSnapshot()
{
	LastAttackZoneSnapshot = GetCurrentStaminaZone();
	if (const UWorld* World = GetWorld())
	{
		LastAttackSnapshotTime = World->GetTimeSeconds();
	}
}

bool AHeroCharacter::ConsumeStamina(int32 Cost, bool bAllowInExhausted)
{
	if (!IsValid(CharacterDataComp))
	{
		return false;
	}
	if (!bAllowInExhausted && GetCurrentStaminaZone() == EStaminaZone::Exhausted)
	{
		return false;
	}
	if (Cost <= 0)
	{
		return true;
	}

	const int32 CurrentMP = CharacterDataComp->GetCurrentMP();
	const int32 NewMP = FMath::Clamp(CurrentMP - Cost, 0, GetTotalStaminaMax());
	CharacterDataComp->SetCurrentMP(NewMP);
	StaminaRegenAccumulator = 0.0f;

	if (const UWorld* World = GetWorld())
	{
		LastStaminaConsumeTime = World->GetTimeSeconds();
	}

	CheckAndBroadcastZoneChange();
	return true;
}

bool AHeroCharacter::TryConsumeDodgeStamina()
{
	if (!CanHeroDodge())
	{
		return false;
	}
	return ConsumeStamina(DodgeStaminaCost, false);
}

void AHeroCharacter::AddBonusStamina(int32 Amount)
{
	if (!IsValid(CharacterDataComp) || Amount <= 0)
	{
		return;
	}
	const int32 TotalMax = GetTotalStaminaMax();
	const int32 NewMP = FMath::Clamp(CharacterDataComp->GetCurrentMP() + Amount, 0, TotalMax);
	CharacterDataComp->SetCurrentMP(NewMP);
	if (NewMP >= GetNormalStaminaCap())
	{
		StaminaRegenAccumulator = 0.0f;
	}
	CheckAndBroadcastZoneChange();
}

bool AHeroCharacter::IsStaminaRegenBlocked() const
{
	if (!IsAlive())
	{
		return true;
	}
	if (bIsDodging)
	{
		return true;
	}
	if (const UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (MoveComp->IsFalling())
		{
			return true;
		}
	}
	if (IsValid(CharacterDataComp))
	{
		// 攻击 / 技能硬直期间不恢复体力
		if (CharacterDataComp->ActionTags.HasTag(FCombatTags::State_LockSkill)
			|| CharacterDataComp->ActionTags.HasTag(FCombatTags::State_LockMove))
		{
			return true;
		}
	}
	return false;
}

void AHeroCharacter::UpdateStaminaRegen(float DeltaTime)
{
	if (!IsValid(CharacterDataComp) || StaminaRegenRate <= 0.0f || DeltaTime <= 0.0f)
	{
		return;
	}

	if (IsStaminaRegenBlocked())
	{
		StaminaRegenAccumulator = 0.0f;
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		if ((World->GetTimeSeconds() - LastStaminaConsumeTime) < StaminaRegenDelay)
		{
			return;
		}
	}

	const int32 NormalCap = GetNormalStaminaCap();
	const int32 CurrentMP = CharacterDataComp->GetCurrentMP();

	// 自然恢复的终点是普通体力上限，不会自然进入活力区，也不会清除已激活的活力区体力
	if (CurrentMP >= NormalCap)
	{
		StaminaRegenAccumulator = 0.0f;
		return;
	}

	StaminaRegenAccumulator += StaminaRegenRate * DeltaTime;
	if (StaminaRegenAccumulator >= 1.0f)
	{
		const int32 WholeGain = FMath::FloorToInt(StaminaRegenAccumulator);
		StaminaRegenAccumulator -= static_cast<float>(WholeGain);
		const int32 NewMP = FMath::Min(CurrentMP + WholeGain, NormalCap);
		CharacterDataComp->SetCurrentMP(NewMP);
		if (NewMP >= NormalCap)
		{
			StaminaRegenAccumulator = 0.0f;
		}
		CheckAndBroadcastZoneChange();
	}
}

void AHeroCharacter::CheckAndBroadcastZoneChange()
{
	const EStaminaZone NewZone = GetCurrentStaminaZone();
	if (NewZone != LastBroadcastZone)
	{
		const EStaminaZone Prev = LastBroadcastZone;
		LastBroadcastZone = NewZone;
		OnStaminaZoneChanged.Broadcast(Prev, NewZone);
	}
}

bool AHeroCharacter::IsDodgeMontage(const UAnimMontage* Montage) const
{
	if (!Montage)
	{
		return false;
	}

	for (const TSoftObjectPtr<UAnimMontage>& Entry : DodgeMontages)
	{
		if (Entry.Get() == Montage)
		{
			return true;
		}
	}

	return Montage->GetName().Contains(TEXT("Dodge"), ESearchCase::IgnoreCase);
}

void AHeroCharacter::HandleMontageStarted(UAnimMontage* Montage)
{
	if (!IsDodgeMontage(Montage))
	{
		return;
	}

	if (!TryConsumeDodgeStamina())
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				AnimInst->Montage_Stop(0.0f, Montage);
			}
		}
		return;
	}

	bIsDodging = true;
	ActiveDodgeMontage = Montage;
}

void AHeroCharacter::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bIsDodging && (ActiveDodgeMontage.Get() == Montage || IsDodgeMontage(Montage)))
	{
		bIsDodging = false;
		ActiveDodgeMontage = nullptr;
	}
}

FString AHeroCharacter::GetSkillDisplayNameByID(int32 SkillID, int32 SkillLevel) const
{
	if (SkillID <= 0)
	{
		return FString();
	}

	if (UTableDataManagerSubsystem* Tables = UTableDataManagerSubsystem::Get())
	{
		if (const TArray<FSkillBaseVo*>* SkillLevels = Tables->SkillBaseMap.Find(SkillID))
		{
			if (SkillLevels->Num() > 0)
			{
				const int32 Index = FMath::Clamp(SkillLevel - 1, 0, SkillLevels->Num() - 1);
				if (const FSkillBaseVo* Vo = (*SkillLevels)[Index])
				{
					return Vo->MagName.TrimStartAndEnd();
				}
			}
		}
	}

	// 回退：当 TableDataManagerSubsystem 尚未初始化时（如编辑器预览或单元测试）直接查 SkillBaseVo 数据表
	if (UDataTable* SkillTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/TableData/SkillBaseVo.SkillBaseVo"))))
	{
		TArray<FSkillBaseVo*> AllRows;
		SkillTable->GetAllRows<FSkillBaseVo>(TEXT("GetSkillDisplayNameByID"), AllRows);
		const FSkillBaseVo* Matched = nullptr;
		for (const FSkillBaseVo* Row : AllRows)
		{
			if (!Row || Row->ID != SkillID)
			{
				continue;
			}
			if (Row->level == SkillLevel)
			{
				return Row->MagName.TrimStartAndEnd();
			}
			if (!Matched)
			{
				Matched = Row;
			}
		}
		if (Matched)
		{
			return Matched->MagName.TrimStartAndEnd();
		}
	}

	return FString();
}

FString AHeroCharacter::GetSkillQDisplayName() const
{
	return GetSkillDisplayNameByID(SkillQ_ID, 1);
}

FString AHeroCharacter::GetSkillEDisplayName() const
{
	return GetSkillDisplayNameByID(SkillE_ID, 1);
}

// ==================== 英雄专属：怒气系统实现 ====================

static float SanitizeHeroRage(float InRage, int32 MaxCharges)
{
	const float Clamped = FMath::Clamp(InRage, 0.0f, static_cast<float>(FMath::Max(1, MaxCharges)));
	const float NearestInt = FMath::RoundToFloat(Clamped);
	// 消除 0.2 * 5 等浮点累加产生的 0.9999999 误差，确保满 5 次普攻稳定激活 1 点
	if (FMath::Abs(Clamped - NearestInt) < 1e-4f)
	{
		return NearestInt;
	}
	return Clamped;
}

float AHeroCharacter::GetCurrentRage() const
{
	return SanitizeHeroRage(CurrentRage, MaxRageCharges);
}

int32 AHeroCharacter::GetAvailableRageCharges() const
{
	const float CleanRage = GetCurrentRage();
	return FMath::Clamp(FMath::FloorToInt(CleanRage + 1e-4f), 0, FMath::Max(1, MaxRageCharges));
}

float AHeroCharacter::GetChargingRageProgress() const
{
	const int32 MaxC = FMath::Max(1, MaxRageCharges);
	const int32 FullCharges = GetAvailableRageCharges();
	if (FullCharges >= MaxC)
	{
		return 0.0f;
	}
	const float Frac = GetCurrentRage() - static_cast<float>(FullCharges);
	return FMath::Clamp(Frac, 0.0f, 1.0f);
}

bool AHeroCharacter::HasEnoughRage(int32 Cost) const
{
	if (Cost <= 0)
	{
		return true;
	}
	return GetAvailableRageCharges() >= Cost;
}

void AHeroCharacter::SetCurrentRage(float NewRage)
{
	const float Sanitized = SanitizeHeroRage(NewRage, MaxRageCharges);
	if (!FMath::IsNearlyEqual(CurrentRage, Sanitized, 1e-5f))
	{
		CurrentRage = Sanitized;
		OnRageChanged.Broadcast(CurrentRage, GetAvailableRageCharges());
	}
	else
	{
		CurrentRage = Sanitized;
	}
}

void AHeroCharacter::AddRage(float Amount)
{
	if (Amount <= 0.0f || !IsAlive())
	{
		return;
	}
	SetCurrentRage(GetCurrentRage() + Amount);
}

bool AHeroCharacter::ConsumeRage(int32 Cost)
{
	if (Cost <= 0)
	{
		return true;
	}
	if (!HasEnoughRage(Cost))
	{
		return false;
	}
	SetCurrentRage(GetCurrentRage() - static_cast<float>(Cost));
	return true;
}

bool AHeroCharacter::IsNormalAttackSkill(const FSkillBaseVo* SkillVo) const
{
	if (!SkillVo)
	{
		return false;
	}
	if (SkillVo->ID == NormalAttackSkillID)
	{
		return true;
	}
	return SkillVo->skillVisualDataID.Equals(TEXT("weapon"), ESearchCase::IgnoreCase);
}

int32 AHeroCharacter::GetSkillRageCostByID(int32 SkillID) const
{
	if (SkillID == NormalAttackSkillID)
	{
		return 0;
	}
	if (const int32* OverrideCost = SkillRageCostOverrides.Find(SkillID))
	{
		return FMath::Max(0, *OverrideCost);
	}
	return FMath::Max(0, DefaultSkillRageCost);
}

int32 AHeroCharacter::GetSkillRageCost(int32 SkillID, const FSkillBaseVo* SkillVo) const
{
	if (IsNormalAttackSkill(SkillVo))
	{
		return 0;
	}
	return GetSkillRageCostByID(SkillID);
}

void AHeroCharacter::OnNormalAttackAccumulateRage()
{
	if (NormalAttackRageGain > 0.0f)
	{
		AddRage(NormalAttackRageGain);
	}
}

// Called every frame
void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 监测运行时或编辑器 Details 面板直接修改 StaminaZoneCapacities 数组
	bool bZoneConfigChanged = (StaminaZoneCapacities.Num() != HeroStaminaDefaults::ZoneCount);
	if (!bZoneConfigChanged)
	{
		for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
		{
			if (FMath::Max(0, StaminaZoneCapacities[i]) != CachedZoneCapacities[i])
			{
				bZoneConfigChanged = true;
				break;
			}
		}
	}
	if (bZoneConfigChanged)
	{
		SyncStaminaZoneConfig();
	}

	UpdateStaminaRegen(DeltaTime);
}
