#include "basic_tps/Core/Loot/LootDropComponent.h"

#include "Engine/World.h"
#include "Loot/LootGeneratorSubsystem.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/MonsterCharacter.h"
#include "basic_tps/Core/Data/MonsterDataComponent.h"
#include "basic_tps/Core/Loot/LootPickupActor.h"
#include "basic_tps/Core/TableData/MonsterBaseVo.h"

DEFINE_LOG_CATEGORY_STATIC(LogLootDropComp, Log, All);

ULootDropComponent::ULootDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickupActorClass = ALootPickupActor::StaticClass();
}

TArray<FString> ULootDropComponent::GetAvailableLootProfileIds()
{
	return ULootGeneratorSubsystem::GetAvailableProfileIdsFromDisk();
}

void ULootDropComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACombatCharacter* CombatOwner = Cast<ACombatCharacter>(GetOwner()))
	{
		DeathDelegateHandle = CombatOwner->OnCharacterDeath.AddUObject(this, &ULootDropComponent::HandleOwnerDeath);
	}
}

void ULootDropComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DeathDelegateHandle.IsValid())
	{
		if (ACombatCharacter* CombatOwner = Cast<ACombatCharacter>(GetOwner()))
		{
			CombatOwner->OnCharacterDeath.Remove(DeathDelegateHandle);
		}
		DeathDelegateHandle.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void ULootDropComponent::HandleOwnerDeath(ACombatCharacter* DeadCharacter)
{
	if (!bEnableDrop || bHasDropped) return;
	TriggerDrop(0.0f, false);
}

void ULootDropComponent::BuildLootContext(float MagicFind, bool bIsFirstKill, FLootContext& OutContext, int32& OutDropId) const
{
	OutContext.MonsterLevel = FMath::Max(1, FallbackMonsterLevel);
	OutContext.BaseGold = FMath::Max(0, FallbackBaseGold);
	OutContext.MonsterTier = bOverrideTier ? TierOverride : EMonsterLootTier::Normal;
	OutContext.MagicFind = MagicFind;
	OutContext.bIsFirstKill = bIsFirstKill;
	OutContext.ExtraGuaranteedDrops = ExtraGuaranteedDrops;
	OutDropId = 0;

	if (const AMonsterCharacter* Monster = Cast<AMonsterCharacter>(GetOwner()))
	{
		if (const UMonsterDataComponent* DataComp = Monster->FindComponentByClass<UMonsterDataComponent>())
		{
			if (const FMonsterBaseVo* Vo = DataComp->baseVo)
			{
				OutContext.MonsterLevel = FMath::Max(1, Vo->Lvl);
				OutContext.BaseGold = FMath::Max(0, Vo->dropMoney);
				OutDropId = Vo->dropID;

				if (!bOverrideTier)
				{
					// 根据 MonsterBaseVo.nameColor 自动识别稀有度阶级（#A335EE=Boss, #0070DD=领主）
					if (Vo->nameColor.Contains(TEXT("A335EE"), ESearchCase::IgnoreCase))
					{
						OutContext.MonsterTier = EMonsterLootTier::Boss;
					}
					else if (Vo->nameColor.Contains(TEXT("0070DD"), ESearchCase::IgnoreCase))
					{
						OutContext.MonsterTier = EMonsterLootTier::Champion;
					}
					else
					{
						OutContext.MonsterTier = EMonsterLootTier::Normal;
					}
				}
			}
		}
	}
}

TArray<FLootDropResult> ULootDropComponent::TriggerDrop(float MagicFind, bool bIsFirstKill)
{
	TArray<FLootDropResult> Results;
	if (!bEnableDrop || bHasDropped) return Results;
	bHasDropped = true;

	ULootGeneratorSubsystem* LootSub = ULootGeneratorSubsystem::Get(this);
	if (!LootSub)
	{
		UE_LOG(LogLootDropComp, Warning, TEXT("触发掉落失败：找不到 ULootGeneratorSubsystem"));
		return Results;
	}

	FLootContext Ctx;
	int32 DropId = 0;
	BuildLootContext(MagicFind, bIsFirstKill, Ctx, DropId);

	const FName ResolvedProfile = LootSub->ResolveProfileId(LootProfileId, DropId, Ctx.MonsterTier);
	LootSub->RollLoot(ResolvedProfile, Ctx, Results);

	if (Results.Num() > 0 && GetOwner())
	{
		const FVector Origin = GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 35.f);
		SpawnDropActorsAt(Origin, Results);
	}

	OnLootDropped.Broadcast(Results);
	return Results;
}

FVector ULootDropComponent::FindGroundPoint(const FVector& Origin, float AngleRad, float Radius) const
{
	const FVector Dir(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);
	const FVector Candidate = Origin + Dir * Radius;

	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(LootGroundTrace), false, GetOwner());
		const FVector Start = Candidate + FVector(0.f, 0.f, 180.f);
		const FVector End = Candidate - FVector(0.f, 0.f, 400.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
		{
			return Hit.ImpactPoint + FVector(0.f, 0.f, 4.f);
		}
	}
	return FVector(Candidate.X, Candidate.Y, Origin.Z - 35.f);
}

void ULootDropComponent::SpawnDropActorsAt(const FVector& Origin, const TArray<FLootDropResult>& Results) const
{
	UWorld* World = GetWorld();
	if (!World || Results.Num() == 0) return;

	UClass* SpawnClass = PickupActorClass ? PickupActorClass.Get() : ALootPickupActor::StaticClass();
	const int32 Count = Results.Num();
	const float BaseAngle = FMath::FRandRange(0.f, TWO_PI);

	for (int32 i = 0; i < Count; ++i)
	{
		// 按圆环均匀扇形错开落点 + 轻微抖动，防止多件装备叠在一起点不到
		const float Angle = BaseAngle + (TWO_PI * static_cast<float>(i) / static_cast<float>(Count)) + FMath::FRandRange(-0.22f, 0.22f);
		const float Radius = FMath::FRandRange(SpreadRadiusMin, FMath::Max(SpreadRadiusMin, SpreadRadiusMax));
		const FVector TargetLoc = FindGroundPoint(Origin, Angle, Radius);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (ALootPickupActor* Pickup = World->SpawnActor<ALootPickupActor>(SpawnClass, Origin, FRotator::ZeroRotator, Params))
		{
			Pickup->InitializeLoot(Results[i], Origin, TargetLoc, LaunchArcHeight + FMath::FRandRange(-12.f, 18.f));
		}
	}
}
