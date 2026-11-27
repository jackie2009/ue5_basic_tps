#include "basic_tps/Core/Loot/LootWorldSubsystem.h"

#include "Engine/World.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Loot/LootGeneratorSubsystem.h"
#include "basic_tps/Core/Loot/LootPickupActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogLootWorld, Log, All);

class FLootPickupInputProcessor : public IInputProcessor
{
public:
	explicit FLootPickupInputProcessor(ULootWorldSubsystem* InOwner) : Owner(InOwner) {}
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		ULootWorldSubsystem* Sub = Owner.Get();
		return Sub && Sub->HandleKeyDown(InKeyEvent.GetKey(), InKeyEvent.IsRepeat());
	}

private:
	TWeakObjectPtr<ULootWorldSubsystem> Owner;
};

bool ULootWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

void ULootWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FLootPickupInputProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
	RegisterConsoleCommands();
}

void ULootWorldSubsystem::Deinitialize()
{
	UnregisterConsoleCommands();
	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
	InputProcessor.Reset();
	ActivePickups.Empty();
	Super::Deinitialize();
}

ULootWorldSubsystem* ULootWorldSubsystem::Get(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<ULootWorldSubsystem>() : nullptr;
}

void ULootWorldSubsystem::RegisterPickup(ALootPickupActor* Pickup)
{
	if (Pickup)
	{
		ActivePickups.AddUnique(Pickup);
	}
}

void ULootWorldSubsystem::UnregisterPickup(ALootPickupActor* Pickup)
{
	ActivePickups.Remove(Pickup);
}

bool ULootWorldSubsystem::HandleKeyDown(const FKey& Key, bool bIsRepeat)
{
	if (bIsRepeat || Key != EKeys::F) return false;
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return false;

	// 当鼠标已显示（例如背包界面打开）时不抢占 F 键
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (PC->bShowMouseCursor) return false;
	}
	return TryPickupNearestForPlayer(260.0f);
}

bool ULootWorldSubsystem::TryPickupNearestForPlayer(float MaxDistance)
{
	UWorld* World = GetWorld();
	APawn* PlayerPawn = World ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;
	if (!PlayerPawn) return false;

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	const float MaxDistSq = FMath::Square(MaxDistance);
	float BestDistSq = MaxDistSq;
	ALootPickupActor* BestPickup = nullptr;

	for (int32 i = ActivePickups.Num() - 1; i >= 0; --i)
	{
		ALootPickupActor* Pickup = ActivePickups[i].Get();
		if (!Pickup || Pickup->IsPickedUp())
		{
			ActivePickups.RemoveAtSwap(i);
			continue;
		}
		if (!Pickup->HasLanded()) continue;

		const float DistSq = FVector::DistSquared(PlayerLoc, Pickup->GetActorLocation());
		if (DistSq <= BestDistSq)
		{
			BestDistSq = DistSq;
			BestPickup = Pickup;
		}
	}

	return BestPickup ? BestPickup->TryPickupBy(PlayerPawn) : false;
}

void ULootWorldSubsystem::SpawnTestLootAtPlayer(FName ProfileId, int32 MonsterLevel, float MagicFind, int32 BaseGold)
{
	UWorld* World = GetWorld();
	APawn* PlayerPawn = World ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;
	ULootGeneratorSubsystem* LootSub = ULootGeneratorSubsystem::Get(World);
	if (!World || !PlayerPawn || !LootSub) return;

	FLootContext Ctx;
	Ctx.MonsterLevel = FMath::Max(1, MonsterLevel);
	Ctx.MagicFind = MagicFind;
	Ctx.BaseGold = BaseGold;
	Ctx.bIsFirstKill = true;

	TArray<FLootDropResult> Results;
	const FName Resolved = LootSub->ResolveProfileId(ProfileId, 0, EMonsterLootTier::Boss);
	LootSub->RollLoot(Resolved, Ctx, Results);

	const FVector Origin = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 140.0f + FVector(0.f, 0.f, 40.f);
	const int32 Count = Results.Num();
	const float BaseAngle = FMath::FRandRange(0.f, TWO_PI);

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = BaseAngle + (TWO_PI * static_cast<float>(i) / FMath::Max(1, Count));
		const float Radius = FMath::FRandRange(80.f, 170.f);
		FVector Target = Origin + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Target + FVector(0.f, 0.f, 180.f), Target - FVector(0.f, 0.f, 400.f), ECC_WorldStatic))
		{
			Target = Hit.ImpactPoint + FVector(0.f, 0.f, 4.f);
		}
		else
		{
			Target.Z = PlayerPawn->GetActorLocation().Z - 80.f;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (ALootPickupActor* Pickup = World->SpawnActor<ALootPickupActor>(ALootPickupActor::StaticClass(), Origin, FRotator::ZeroRotator, Params))
		{
			Pickup->InitializeLoot(Results[i], Origin, Target, 100.f);
		}
	}

	UE_LOG(LogLootWorld, Display, TEXT("[Loot.SpawnHere] Profile=%s -> 生成 %d 件掉落物（走近按 F 或点击拾取）"),
		*Resolved.ToString(), Results.Num());
}

void ULootWorldSubsystem::RegisterConsoleCommands()
{
	UnregisterConsoleCommands();
	TWeakObjectPtr<ULootWorldSubsystem> WeakThis(this);

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Loot.SpawnHere"),
		TEXT("在玩家前方爆一地装备测试表现: Loot.SpawnHere [ProfileId=Boss_SkeletonKing] [Level=35] [MF=0] [BaseGold=1000]"),
		FConsoleCommandWithArgsDelegate::CreateLambda([WeakThis](const TArray<FString>& Args)
		{
			ULootWorldSubsystem* Self = WeakThis.Get();
			if (!Self) return;
			const FName ProfileId = Args.Num() >= 1 ? FName(*Args[0]) : FName(TEXT("Boss_SkeletonKing"));
			const int32 Lvl = Args.Num() >= 2 ? FCString::Atoi(*Args[1]) : 35;
			const float MF = Args.Num() >= 3 ? FCString::Atof(*Args[2]) : 0.0f;
			const int32 Gold = Args.Num() >= 4 ? FCString::Atoi(*Args[3]) : 1000;
			Self->SpawnTestLootAtPlayer(ProfileId, Lvl, MF, Gold);
		}),
		ECVF_Default));
}

void ULootWorldSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleObject* Cmd : ConsoleCommands)
	{
		if (Cmd)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Cmd);
		}
	}
	ConsoleCommands.Empty();
}
