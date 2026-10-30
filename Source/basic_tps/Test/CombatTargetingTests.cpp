#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Targeting/CharacterTargetableComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetRegistry.h"
#include "basic_tps/Core/Utils/CombatFacingUtils.h"

namespace CombatTargetingTests
{
struct FTargetTestScene
{
	UWorld* World;
	FTargetTestScene()
	{
		const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false)
			.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
		World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();
		World->SetBegunPlay(true);
	}
	~FTargetTestScene()
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	ACombatCharacter* Character(const FVector& Location, ECombatFaction Faction)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto* Actor = World->SpawnActor<ACombatCharacter>(Location, FRotator::ZeroRotator, Params);
		Actor->CharacterDataComp->Faction = Faction;
		Actor->GetCharacterMovement()->DisableMovement();
		if (!Actor->HasActorBegunPlay()) Actor->DispatchBeginPlay();
		return Actor;
	}
	void Advance(float Seconds) { World->Tick(LEVELTICK_All, Seconds); }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTargetLifecycle, "BasicTPS.Targeting.LifecycleAndVisibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTargetLifecycle::RunTest(const FString& Parameters)
{
	using namespace CombatTargetingTests;
	FTargetTestScene Scene;
	auto* Hero = Scene.Character(FVector::ZeroVector, ECombatFaction::Hero);
	auto* Enemy = Scene.Character(FVector(1000, 0, 0), ECombatFaction::Monster);
	auto* Friend = Scene.Character(FVector(600, 0, 0), ECombatFaction::Pet);
	auto* Selector = Hero->TargetingComp.Get();
	TestFalse(TEXT("Self cannot be selected"), Selector->SetSelectedTarget(Hero));
	TestFalse(TEXT("Pets are allied to hero"), Selector->SetSelectedTarget(Friend));
	TestTrue(TEXT("Enemy can be selected without a player camera (AI path)"), Selector->SetSelectedTarget(Enemy));
	TestEqual(TEXT("Action target matches selection"), Selector->GetActionTarget(), static_cast<AActor*>(Enemy));
	auto* Wall = Scene.World->SpawnActor<AActor>();
	auto* Box = NewObject<UBoxComponent>(Wall);
	Wall->SetRootComponent(Box);
	Box->SetBoxExtent(FVector(20, 200, 200));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Block);
	Box->RegisterComponent();
	Wall->SetActorLocation(FVector(500, 0, 0));
	TestNull(TEXT("Blocked action target cannot be used by a skill"), Selector->GetActionTarget());
	TestFalse(TEXT("Cannot manually acquire through a wall"), Selector->SetSelectedTarget(Enemy));
	Wall->Destroy();
	Enemy->TargetableComp->SetTargetable(false);
	TestNull(TEXT("Disabling target immediately invalidates selection"), Selector->GetSelectedTarget());
	Enemy->TargetableComp->SetTargetable(true);
	TestTrue(TEXT("Re-enabled target selectable"), Selector->SetSelectedTarget(Enemy));
	Selector->Deactivate();
	TestNull(TEXT("Deactivating selection clears state"), Selector->GetSelectedTarget());
	TestNull(TEXT("Inactive selection cannot supply an action target"), Selector->GetActionTarget());
	Selector->Activate();
	Selector->SetSelectedTarget(Enemy);
	Enemy->CharacterDataComp->SetCurrentHP(0);
	TestNull(TEXT("HP change is revalidated at action time"), Selector->GetActionTarget());
	Selector->RefreshTarget();
	TestNull(TEXT("Dead target cleared on update even without death event"), Selector->GetSelectedTarget());
	Enemy->CharacterDataComp->SetCurrentHP(100);
	Selector->SetSelectedTarget(Enemy);
	Enemy->Destroy();
	TestNull(TEXT("Destroyed target clears safely"), Selector->GetSelectedTarget());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTargetSwitching, "BasicTPS.Targeting.AutomaticSwitching",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTargetSwitching::RunTest(const FString& Parameters)
{
	using namespace CombatTargetingTests;
	FTargetTestScene Scene;
	auto* Hero = Scene.Character(FVector::ZeroVector, ECombatFaction::Hero);
	auto* PC = Scene.World->SpawnActor<APlayerController>();
	PC->SetAsLocalPlayerController();
	PC->Possess(Hero);
	auto* First = Scene.Character(FVector(1000, 100, 0), ECombatFaction::Monster);
	auto* Second = Scene.Character(FVector(1000, 450, 0), ECombatFaction::Monster);
	auto* Selector = Hero->TargetingComp.Get();
	Selector->bAutoSelect = true;
	Selector->Settings.Direction = ECombatTargetingDirection::Movement;
	Selector->RefreshTarget();
	TestEqual(TEXT("Initial acquisition favours directional intent"), Selector->GetSelectedTarget(), static_cast<AActor*>(First));
	First->SetActorLocation(FVector(1000, 480, 0));
	Second->SetActorLocation(FVector(1000, 0, 0));
	Selector->RefreshTarget();
	TestEqual(TEXT("Better challenger does not cause an instant flicker"), Selector->GetSelectedTarget(), static_cast<AActor*>(First));
	Scene.Advance(0.1f);
	Selector->RefreshTarget();
	TestEqual(TEXT("Dwell time not yet satisfied"), Selector->GetSelectedTarget(), static_cast<AActor*>(First));
	Scene.Advance(0.1f);
	Selector->RefreshTarget();
	TestEqual(TEXT("Sustained advantage switches target"), Selector->GetSelectedTarget(), static_cast<AActor*>(Second));
	Selector->SetTargetLocked(true);
	First->SetActorLocation(FVector(900, 0, 0));
	Second->SetActorLocation(FVector(1000, 500, 0));
	Scene.Advance(0.3f);
	Selector->RefreshTarget();
	TestEqual(TEXT("Explicit lock survives a better candidate"), Selector->GetSelectedTarget(), static_cast<AActor*>(Second));
	auto* Wall = Scene.World->SpawnActor<AActor>();
	auto* Box = NewObject<UBoxComponent>(Wall);
	Wall->SetRootComponent(Box);
	Box->SetBoxExtent(FVector(20, 80, 150));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Block);
	Box->RegisterComponent();
	Wall->SetActorLocation(FVector(500, 250, 50));
	Selector->RefreshTarget();
	TestEqual(TEXT("Occlusion grace retains selected identity"), Selector->GetSelectedTarget(), static_cast<AActor*>(Second));
	TestNull(TEXT("Occlusion grace does not expose a usable action target"), Selector->GetActionTarget());
	Scene.Advance(0.25f);
	Selector->RefreshTarget();
	TestEqual(TEXT("Expired occlusion grace selects visible alternative"), Selector->GetSelectedTarget(), static_cast<AActor*>(First));
	TestFalse(TEXT("Occlusion loss releases explicit lock"), Selector->IsTargetLocked());
	Wall->Destroy();
	Selector->SetSelectedTarget(Second);
	Selector->SetTargetLocked(true);
	Second->TargetableComp->SetTargetable(false);
	TestFalse(TEXT("Invalidation clears lock"), Selector->IsTargetLocked());
	Selector->RefreshTarget();
	TestEqual(TEXT("Invalid target immediately replaced on next update"), Selector->GetSelectedTarget(), static_cast<AActor*>(First));
	First->SetActorLocation(FVector(2900, 0, 0));
	Selector->RefreshTarget();
	TestNull(TEXT("Beyond retention range clears target"), Selector->GetSelectedTarget());
	PC->UnPossess();
	Selector->RefreshTarget();
	TestNull(TEXT("No local controller means no player auto-selection"), Selector->GetSelectedTarget());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTargetFacing, "BasicTPS.Targeting.SharedFacing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTargetFacing::RunTest(const FString& Parameters)
{
	using namespace CombatTargetingTests;
	FTargetTestScene Scene;
	auto* Hero = Scene.Character(FVector::ZeroVector, ECombatFaction::Hero);
	auto* Enemy = Scene.Character(FVector(0, 1000, 0), ECombatFaction::Monster);
	TestFalse(TEXT("Empty selection is a no-op"), UCombatFacingUtils::FaceToSelectedTargetImmediately(Hero));
	Hero->TargetingComp->SetSelectedTarget(Enemy);
	TestTrue(TEXT("Hero can face selected enemy"), UCombatFacingUtils::FaceToSelectedTargetImmediately(Hero));
	TestTrue(TEXT("Yaw points to enemy"), FMath::IsNearlyEqual(Hero->GetActorRotation().Yaw, 90.f, 0.1f));
	Enemy->TargetingComp->SetSelectedTarget(Hero);
	TestTrue(TEXT("Enemy uses the same public function"), UCombatFacingUtils::FaceToSelectedTargetImmediately(Enemy));
	TestTrue(TEXT("Enemy yaw points back to hero"), FMath::IsNearlyEqual(Enemy->GetActorRotation().Yaw, -90.f, 0.1f));
	TestFalse(TEXT("Null snapshot is safe"), UCombatFacingUtils::FaceToTargetImmediately(Hero, nullptr));
	TestFalse(TEXT("Cannot face self"), UCombatFacingUtils::FaceToTargetImmediately(Hero, Hero));
	Enemy->TargetableComp->SetTargetable(false);
	TestFalse(TEXT("Invalid captured enemy is rejected"), UCombatFacingUtils::FaceToTargetImmediately(Hero, Enemy));
	return true;
}
#endif
