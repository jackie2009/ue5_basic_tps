#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Data/ItemDatabaseSubsystem.h"
#include "Data/ItemSystemTypes.h"
#include "Inventory/InventoryGridComponent.h"
#include "Loot/LootGeneratorSubsystem.h"
#include "Loot/LootTypes.h"
#include "basic_tps/Core/Character/HeroCharacter.h"
#include "basic_tps/Core/Character/MonsterCharacter.h"
#include "basic_tps/Core/Loot/LootDropComponent.h"
#include "basic_tps/Core/Loot/LootPickupActor.h"
#include "Kismet/GameplayStatics.h"

namespace LootSystemTestHelpers
{
struct FLootTestEnv
{
	UWorld* World = nullptr;
	UGameInstance* GI = nullptr;

	FLootTestEnv()
	{
		GI = NewObject<UGameInstance>(GEngine);
		GI->Init();

		const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false)
			.CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
		World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
		World->SetGameInstance(GI);
		FWorldContext& Ctx = GEngine->CreateNewWorldContext(EWorldType::Game);
		Ctx.SetCurrentWorld(World);
		Ctx.OwningGameInstance = GI;
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();
		World->SetBegunPlay(true);
	}

	~FLootTestEnv()
	{
		if (World)
		{
			World->EndPlay(EEndPlayReason::Quit);
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
		if (GI)
		{
			GI->Shutdown();
		}
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootAffixAndConfigTest, "BasicTPS.Loot.AffixesAndConfigs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLootAffixAndConfigTest::RunTest(const FString& Parameters)
{
	using namespace LootSystemTestHelpers;
	FLootTestEnv Env;

	UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(Env.World);
	ULootGeneratorSubsystem* Loot = ULootGeneratorSubsystem::Get(Env.World);
	TestNotNull(TEXT("UItemDatabaseSubsystem should exist"), DB);
	TestNotNull(TEXT("ULootGeneratorSubsystem should exist"), Loot);
	if (!DB || !Loot) return false;

	// 1. 道具表 & 词缀表加载校验
	TestTrue(TEXT("ItemBase.csv loaded"), DB->GetAllRows().Num() >= 20);
	TestEqual(TEXT("RandomAffixes.json loaded 242 rows"), DB->GetAllAffixes().Num(), 242);

	const TArray<FString> ConfigErrors = Loot->ValidateConfigs();
	TestEqual(TEXT("Loot JSON configs have zero validation errors"), ConfigErrors.Num(), 0);
	for (const FString& Err : ConfigErrors)
	{
		AddError(Err);
	}

	// 2. 编辑器下拉选项读取校验
	const TArray<FString> DiskProfiles = ULootGeneratorSubsystem::GetAvailableProfileIdsFromDisk();
	TestTrue(TEXT("Disk profiles contain Boss_SkeletonKing"), DiskProfiles.Contains(TEXT("Boss_SkeletonKing")));
	TestTrue(TEXT("Disk profiles contain Default_Normal"), DiskProfiles.Contains(TEXT("Default_Normal")));

	// 3. 品质与随机词缀生成规则校验
	FRandomStream Rng(42);
	const FItemInstance NormalSword = DB->CreateRolledInstance(201, 0, Rng);
	TestEqual(TEXT("Normal item has 0 affixes"), NormalSword.DynamicAttributes.Num(), 0);

	const FItemInstance MagicHelm = DB->CreateRolledInstance(402, 1, Rng);
	TestTrue(TEXT("Magic item has 1-2 affixes"), MagicHelm.DynamicAttributes.Num() >= 1 && MagicHelm.DynamicAttributes.Num() <= 2);

	const FItemInstance RareSword = DB->CreateRolledInstance(201, 2, Rng);
	TestEqual(TEXT("Rare weapon has 3 affixes"), RareSword.DynamicAttributes.Num(), 3);
	TestTrue(TEXT("Rare weapon guarantees AttackPercent (7020)"), RareSword.DynamicAttributes.Contains(7020));

	const FItemInstance UniqueNeck = DB->CreateRolledInstance(32009, 0, Rng);
	TestEqual(TEXT("Unique item enforces QualityMin=3"), UniqueNeck.Quality, 3);
	TestTrue(TEXT("Unique item has >= 4 affixes including ExtraAttributes"), UniqueNeck.DynamicAttributes.Num() >= 4);
	TestTrue(TEXT("Unique necklace 32009 has HpMax ExtraAttribute (8001)"), UniqueNeck.DynamicAttributes.Contains(8001));

	// 4. 3 级 Profile 解析优先级
	TestEqual(TEXT("Explicit profile wins"),
		Loot->ResolveProfileId(FName(TEXT("Mob_Goblin")), 38, EMonsterLootTier::Boss), FName(TEXT("Mob_Goblin")));
	TestEqual(TEXT("DropIdMap resolves 38 -> Boss_SkeletonKing"),
		Loot->ResolveProfileId(NAME_None, 38, EMonsterLootTier::Normal), FName(TEXT("Boss_SkeletonKing")));
	TestEqual(TEXT("Tier fallback resolves Boss -> Default_Boss"),
		Loot->ResolveProfileId(NAME_None, 99999, EMonsterLootTier::Boss), FName(TEXT("Default_Boss")));

	// 5. 模拟 1000 次击杀：Boss 与 MF 效果验证
	FLootContext BossCtx;
	BossCtx.MonsterLevel = 35;
	BossCtx.MonsterTier = EMonsterLootTier::Boss;
	BossCtx.BaseGold = 3000;
	BossCtx.MagicFind = 0.0f;
	const FLootSimulationReport Rep0 = Loot->SimulateProfile(FName(TEXT("Boss_SkeletonKing")), 500, BossCtx, 1001);
	TestTrue(TEXT("Boss drops >= 4 items per kill on average"), static_cast<float>(Rep0.TotalItems) / Rep0.KillCount >= 4.0f);
	TestEqual(TEXT("Boss drops zero normal (white) items"), Rep0.QualityCounts[0], 0);
	TestTrue(TEXT("Boss drops gold"), Rep0.TotalGold > 0);

	BossCtx.MagicFind = 300.0f;
	const FLootSimulationReport RepMF = Loot->SimulateProfile(FName(TEXT("Boss_SkeletonKing")), 500, BossCtx, 1001);
	const float HighQ0 = static_cast<float>(Rep0.QualityCounts[2] + Rep0.QualityCounts[3]) / FMath::Max(1, Rep0.TotalItems);
	const float HighQMF = static_cast<float>(RepMF.QualityCounts[2] + RepMF.QualityCounts[3]) / FMath::Max(1, RepMF.TotalItems);
	TestTrue(TEXT("MF=300% increases Rare+Unique share"), HighQMF > HighQ0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootMonsterDeathAndPickupTest, "BasicTPS.Loot.MonsterDeathAndPickup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLootMonsterDeathAndPickupTest::RunTest(const FString& Parameters)
{
	using namespace LootSystemTestHelpers;
	FLootTestEnv Env;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMonsterCharacter* Monster = Env.World->SpawnActor<AMonsterCharacter>(FVector(300.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
	TestNotNull(TEXT("Monster spawned"), Monster);
	if (!Monster) return false;

	TestNotNull(TEXT("Monster has default LootDropComp"), Monster->LootDropComp.Get());
	Monster->LootDropComp->LootProfileId = FName(TEXT("Boss_SkeletonKing"));
	Monster->LootDropComp->FallbackMonsterLevel = 35;
	Monster->LootDropComp->FallbackBaseGold = 500;
	if (!Monster->HasActorBegunPlay())
	{
		Monster->DispatchBeginPlay();
	}

	// 触发怪物死亡 -> 自动通过 OnCharacterDeath 生成地面掉落物
	Monster->SelfOnDead();

	TArray<AActor*> Pickups;
	UGameplayStatics::GetAllActorsOfClass(Env.World, ALootPickupActor::StaticClass(), Pickups);
	TestTrue(TEXT("Monster death spawned >= 4 ALootPickupActor"), Pickups.Num() >= 4);

	// 创建玩家角色 + PlayerController 拾取装备与金币进背包
	APlayerController* PC = Env.World->SpawnActor<APlayerController>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	AHeroCharacter* Hero = Env.World->SpawnActor<AHeroCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (PC && Hero)
	{
		PC->Possess(Hero);
		int32 PickedCount = 0;
		for (AActor* Act : Pickups)
		{
			if (ALootPickupActor* Pickup = Cast<ALootPickupActor>(Act))
			{
				if (Pickup->TryPickupBy(Hero))
				{
					PickedCount++;
				}
			}
		}
		TestEqual(TEXT("All spawned pickups collected"), PickedCount, Pickups.Num());

		UInventoryGridComponent* Bag = PC->FindComponentByClass<UInventoryGridComponent>();
		TestNotNull(TEXT("PlayerController owns UInventoryGridComponent"), Bag);
		if (Bag)
		{
			TestTrue(TEXT("Bag received dropped items"), Bag->GetEntries().Num() >= 3);
			TestTrue(TEXT("Bag received dropped gold"), Bag->Gold > 0);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
