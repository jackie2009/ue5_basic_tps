#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/HeroCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/CombatTypes.h"
#include "basic_tps/Core/Data/HeroStaminaTypes.h"
#include "basic_tps/Core/Effect/FVfxSpawnConfig.h"
#include "basic_tps/Core/NotifyState/CombatGameplayTags.h"
#include "basic_tps/Core/UI/CombatHUDWidget.h"
#include "basic_tps/Core/Utils/CombatCalculator.h"

namespace HeroStaminaTests
{
struct FStaminaTestScene
{
	UWorld* World = nullptr;
	FStaminaTestScene()
	{
		const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false)
			.CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
		World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();
		World->SetBegunPlay(true);
	}
	~FStaminaTestScene()
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	AHeroCharacter* SpawnHero()
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AHeroCharacter* Hero = World->SpawnActor<AHeroCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
		Hero->CharacterDataComp->Attributes[AttributeEnum::HP] = 100;
		Hero->CharacterDataComp->SetCurrentHP(100);
		Hero->GetCharacterMovement()->DisableMovement();
		Hero->SyncStaminaZoneConfig();
		Hero->CharacterDataComp->SetCurrentMP(Hero->GetNormalStaminaCap());
		if (!Hero->HasActorBegunPlay())
		{
			Hero->DispatchBeginPlay();
		}
		return Hero;
	}
	void Advance(float Seconds)
	{
		const int32 Steps = FMath::Max(1, FMath::CeilToInt(Seconds / 0.05f));
		const float StepDt = Seconds / static_cast<float>(Steps);
		for (int32 i = 0; i < Steps; ++i)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, StepDt);
		}
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeroStaminaZonesAndRules, "BasicTPS.Stamina.ZonesGatingAndDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHeroStaminaZonesAndRules::RunTest(const FString& Parameters)
{
	using namespace HeroStaminaTests;
	FStaminaTestScene Scene;
	AHeroCharacter* Hero = Scene.SpawnHero();

	// 1. 属性仅存在于 Hero，不在基础战斗角色类上
	TestNull(TEXT("ACombatCharacter must not own StaminaZoneCapacities"),
		ACombatCharacter::StaticClass()->FindPropertyByName(TEXT("StaminaZoneCapacities")));
	TestNotNull(TEXT("AHeroCharacter must own StaminaZoneCapacities"),
		AHeroCharacter::StaticClass()->FindPropertyByName(TEXT("StaminaZoneCapacities")));

	// 2. 默认 4 位数组：20 30 50 20
	TestEqual(TEXT("Exhausted cap default"), Hero->GetStaminaZoneCapacity(EStaminaZone::Exhausted), 20);
	TestEqual(TEXT("Fatigued cap default"), Hero->GetStaminaZoneCapacity(EStaminaZone::Fatigued), 30);
	TestEqual(TEXT("Normal cap default"), Hero->GetStaminaZoneCapacity(EStaminaZone::Normal), 50);
	TestEqual(TEXT("Vigorous cap default"), Hero->GetStaminaZoneCapacity(EStaminaZone::Vigorous), 20);
	TestEqual(TEXT("Normal stamina cap"), Hero->GetNormalStaminaCap(), 100);
	TestEqual(TEXT("Total stamina max"), Hero->GetTotalStaminaMax(), 120);
	TestEqual(TEXT("MaxMP synced to total stamina max"), Hero->CharacterDataComp->GetMaxMP(), 120);

	// 3. 区间判定与行为限制
	Hero->CharacterDataComp->SetCurrentMP(10);
	TestEqual(TEXT("10 is Exhausted"), Hero->GetCurrentStaminaZone(), EStaminaZone::Exhausted);
	TestFalse(TEXT("Cannot attack in Exhausted"), Hero->CanHeroAttack());
	TestFalse(TEXT("Cannot jump in Exhausted"), Hero->CanHeroJump());
	TestFalse(TEXT("Cannot dodge in Exhausted"), Hero->CanHeroDodge());

	Hero->CharacterDataComp->SetCurrentMP(20);
	TestEqual(TEXT("20 is still Exhausted boundary"), Hero->GetCurrentStaminaZone(), EStaminaZone::Exhausted);

	Hero->CharacterDataComp->SetCurrentMP(21);
	TestEqual(TEXT("21 enters Fatigued"), Hero->GetCurrentStaminaZone(), EStaminaZone::Fatigued);
	TestTrue(TEXT("Can attack in Fatigued"), Hero->CanHeroAttack());
	TestTrue(TEXT("Can jump in Fatigued"), Hero->CanHeroJump());
	TestTrue(TEXT("Can dodge in Fatigued"), Hero->CanHeroDodge());

	Hero->CharacterDataComp->SetCurrentMP(100);
	TestEqual(TEXT("100 is Normal (not Vigorous)"), Hero->GetCurrentStaminaZone(), EStaminaZone::Normal);

	Hero->AddBonusStamina(15);
	TestEqual(TEXT("115 is Vigorous"), Hero->GetCurrentStaminaZone(), EStaminaZone::Vigorous);

	// 4. 伤害倍率结算：疲劳 70%，普通 100%，活力 115%
	FCombatResult FatiguedRes;
	FatiguedRes.Attacker = Hero;
	FatiguedRes.FinalDamage = 100;
	Hero->CharacterDataComp->SetCurrentMP(30);
	Hero->CaptureAttackStaminaZoneSnapshot();
	Hero->ConsumeStamina(20); // 跌入无力区，但出手瞬间在疲劳区
	UCombatCalculator::AdjustFinalDamage(FatiguedRes);
	TestEqual(TEXT("Fatigued attack deals 70%"), FatiguedRes.FinalDamage, 70);

	FCombatResult NormalRes;
	NormalRes.Attacker = Hero;
	NormalRes.FinalDamage = 100;
	Hero->CharacterDataComp->SetCurrentMP(80);
	Hero->CaptureAttackStaminaZoneSnapshot();
	UCombatCalculator::AdjustFinalDamage(NormalRes);
	TestEqual(TEXT("Normal attack deals 100%"), NormalRes.FinalDamage, 100);

	FCombatResult VigorousRes;
	VigorousRes.Attacker = Hero;
	VigorousRes.FinalDamage = 100;
	Hero->CharacterDataComp->SetCurrentMP(115);
	Hero->CaptureAttackStaminaZoneSnapshot();
	Hero->ConsumeStamina(20); // 跌入普通区，但出手瞬间在活力区
	UCombatCalculator::AdjustFinalDamage(VigorousRes);
	TestEqual(TEXT("Vigorous attack deals 115%"), VigorousRes.FinalDamage, 115);

	// 5. 自然恢复：在攻击锁定时暂停，解除后恢复并停在普通上限 100（不自动进活力区）
	Hero->CharacterDataComp->SetCurrentMP(40);
	Hero->ConsumeStamina(10); // -> 30
	Hero->CharacterDataComp->ActionTags.AddTag(FCombatTags::State_LockSkill);
	Scene.Advance(2.0f);
	TestEqual(TEXT("No regen while State_LockSkill active"), Hero->CharacterDataComp->GetCurrentMP(), 30);

	Hero->CharacterDataComp->ActionTags.RemoveTag(FCombatTags::State_LockSkill);
	Hero->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	Scene.Advance(1.5f);
	TestEqual(TEXT("No regen while jumping/falling"), Hero->CharacterDataComp->GetCurrentMP(), 30);

	Hero->GetCharacterMovement()->DisableMovement();
	Scene.Advance(6.0f);
	TestEqual(TEXT("Natural regen stops at NormalCap 100"), Hero->CharacterDataComp->GetCurrentMP(), 100);
	TestEqual(TEXT("Zone stays Normal after natural regen"), Hero->GetCurrentStaminaZone(), EStaminaZone::Normal);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeroStaminaHudProportions, "BasicTPS.HUD.DynamicStaminaZoneProportions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHeroStaminaHudProportions::RunTest(const FString& Parameters)
{
	using namespace HeroStaminaTests;
	FStaminaTestScene Scene;
	AHeroCharacter* Hero = Scene.SpawnHero();

	UCombatHUDWidget* Hud = CreateWidget<UCombatHUDWidget>(Scene.World, UCombatHUDWidget::StaticClass());
	TestNotNull(TEXT("UCombatHUDWidget created"), Hud);
	if (!Hud)
	{
		return false;
	}

	Hero->CharacterDataComp->SetCurrentMP(75);
	Hud->RefreshHUDFromHero(Hero);

	// 默认比例 20 : 30 : 50 : 20（50段长于20段）
	TestEqual(TEXT("Default Exhausted slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Exhausted), 20.0f);
	TestEqual(TEXT("Default Fatigued slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Fatigued), 30.0f);
	TestEqual(TEXT("Default Normal slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Normal), 50.0f);
	TestEqual(TEXT("Default Vigorous slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Vigorous), 20.0f);
	TestTrue(TEXT("Normal(50) segment wider than Exhausted(20)"),
		Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Normal) > Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Exhausted));

	// MP=75 -> 无力(0..20)=1.0, 疲劳(20..50)=1.0, 普通(50..100)=0.5, 活力(100..120)=0.0
	TestEqual(TEXT("Exhausted fill at 75"), Hud->GetAppliedStaminaZonePercent(EStaminaZone::Exhausted), 1.0f);
	TestEqual(TEXT("Fatigued fill at 75"), Hud->GetAppliedStaminaZonePercent(EStaminaZone::Fatigued), 1.0f);
	TestEqual(TEXT("Normal fill at 75"), Hud->GetAppliedStaminaZonePercent(EStaminaZone::Normal), 0.5f);
	TestEqual(TEXT("Vigorous fill at 75"), Hud->GetAppliedStaminaZonePercent(EStaminaZone::Vigorous), 0.0f);

	// 修改参数后，段长度比例随之动态变化
	Hero->SetStaminaZoneCapacities({ 10, 40, 80, 30 });
	Hud->RefreshHUDFromHero(Hero);
	TestEqual(TEXT("Updated TotalMax"), Hero->GetTotalStaminaMax(), 160);
	TestEqual(TEXT("Updated Exhausted slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Exhausted), 10.0f);
	TestEqual(TEXT("Updated Fatigued slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Fatigued), 40.0f);
	TestEqual(TEXT("Updated Normal slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Normal), 80.0f);
	TestEqual(TEXT("Updated Vigorous slot weight"), Hud->GetAppliedStaminaZoneSlotWeight(EStaminaZone::Vigorous), 30.0f);

	// 右上角动态键位提示：Q/E 读取 SkillBaseVo 技能名，Shift=闪避，Space=跳跃
	TestEqual(TEXT("Hint Q reads skill 100001 name"), Hud->GetHintSkillQText(), FString(TEXT("盾牌冲锋")));
	TestEqual(TEXT("Hint E reads skill 200010 name"), Hud->GetHintSkillEText(), FString(TEXT("盾牌投掷")));
	TestEqual(TEXT("Hint Shift is Dodge"), Hud->GetHintDodgeText(), FString(TEXT("闪避")));
	TestEqual(TEXT("Hint Space is Jump"), Hud->GetHintJumpText(), FString(TEXT("跳跃")));

	// 切换英雄技能槽 ID 后，HUD 动态更新技能名称
	Hero->SkillQ_ID = 200001; // 投掷冰弹
	Hero->SkillE_ID = 200002; // 寒冰盾
	Hud->RefreshHUDFromHero(Hero);
	TestEqual(TEXT("Hint Q updates dynamically when SkillQ_ID changes"), Hud->GetHintSkillQText(), FString(TEXT("投掷冰弹")));
	TestEqual(TEXT("Hint E updates dynamically when SkillE_ID changes"), Hud->GetHintSkillEText(), FString(TEXT("寒冰盾")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeroRageSystemAndHud, "BasicTPS.Rage.HeroAccumulationCostAndHudDisplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHeroRageSystemAndHud::RunTest(const FString& Parameters)
{
	using namespace HeroStaminaTests;
	FStaminaTestScene Scene;
	AHeroCharacter* Hero = Scene.SpawnHero();

	// 1. 怒气系统仅在英雄上，不在共同父类 ACombatCharacter 上
	TestNull(TEXT("ACombatCharacter must not own CurrentRage"),
		ACombatCharacter::StaticClass()->FindPropertyByName(TEXT("CurrentRage")));
	TestNotNull(TEXT("AHeroCharacter must own CurrentRage"),
		AHeroCharacter::StaticClass()->FindPropertyByName(TEXT("CurrentRage")));

	// 2. 初始为 0，现有技能默认消耗 1 点，普攻不消耗怒气
	TestEqual(TEXT("Initial rage is 0"), Hero->GetCurrentRage(), 0.0f);
	TestEqual(TEXT("Initial available rage charges is 0"), Hero->GetAvailableRageCharges(), 0);
	TestEqual(TEXT("Normal attack rage cost is 0"), Hero->GetSkillRageCostByID(100003), 0);
	TestEqual(TEXT("Skill Q (100001) rage cost is 1"), Hero->GetSkillRageCostByID(100001), 1);
	TestEqual(TEXT("Skill E (200010) rage cost is 1"), Hero->GetSkillRageCostByID(200010), 1);
	TestFalse(TEXT("Cannot consume 1 rage when at 0.8"), Hero->ConsumeRage(1));

	// 3. 每次普通攻击积累 0.2：前 4 次均处于积累中（0 点可用），第 5 次正好激活 1 点
	for (int32 Hit = 1; Hit <= 4; ++Hit)
	{
		Hero->OnNormalAttackAccumulateRage();
		TestEqual(TEXT("Still 0 activated charges before 5th hit"), Hero->GetAvailableRageCharges(), 0);
		TestTrue(TEXT("Charging progress matches Hit * 0.2"),
			FMath::IsNearlyEqual(Hero->GetChargingRageProgress(), Hit * 0.2f, 1e-3f));
	}
	TestFalse(TEXT("0.8 rage is not enough for 1-cost skill"), Hero->HasEnoughRage(1));

	Hero->OnNormalAttackAccumulateRage(); // 第 5 次 -> 1.0
	TestEqual(TEXT("5th normal attack activates 1 full rage charge"), Hero->GetAvailableRageCharges(), 1);
	TestTrue(TEXT("Charging progress resets to 0 on exact integer"),
		FMath::IsNearlyEqual(Hero->GetChargingRageProgress(), 0.0f, 1e-3f));
	TestTrue(TEXT("1.0 rage is enough for 1-cost skill"), Hero->HasEnoughRage(1));

	// 4. 再积累 0.4 -> 1.4，消耗 1 点技能怒气后剩余 0.4（保留积累进度）
	Hero->OnNormalAttackAccumulateRage();
	Hero->OnNormalAttackAccumulateRage();
	TestEqual(TEXT("1.4 has 1 full charge"), Hero->GetAvailableRageCharges(), 1);
	TestTrue(TEXT("1.4 has 0.4 charging"), FMath::IsNearlyEqual(Hero->GetChargingRageProgress(), 0.4f, 1e-3f));

	TestTrue(TEXT("Consume 1 rage succeeds at 1.4"), Hero->ConsumeRage(1));
	TestEqual(TEXT("After spending 1 rage from 1.4, 0 full charges remain"), Hero->GetAvailableRageCharges(), 0);
	TestTrue(TEXT("Partial charging progress 0.4 preserved"), FMath::IsNearlyEqual(Hero->GetChargingRageProgress(), 0.4f, 1e-3f));

	// 5. 普攻命中伤害管线集成测试：仅存活敌人受击时 +0.2，敌人不会获得怒气
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACombatCharacter* Enemy = Scene.World->SpawnActor<ACombatCharacter>(FVector(200, 0, 0), FRotator::ZeroRotator, SpawnParams);
	Enemy->CharacterDataComp->Faction = ECombatFaction::Monster;
	Enemy->CharacterDataComp->Attributes[AttributeEnum::HP] = 200;
	Enemy->CharacterDataComp->SetCurrentHP(200);
	Enemy->GetCharacterMovement()->DisableMovement();
	if (!Enemy->HasActorBegunPlay())
	{
		Enemy->DispatchBeginPlay();
	}

	FSkillBaseVo NormalAttackVo;
	NormalAttackVo.ID = 100003;
	NormalAttackVo.skillVisualDataID = TEXT("weapon");
	NormalAttackVo.EffectType = SkillEffectTypeEnum::DC;
	NormalAttackVo.Power = { 20 };

	FEffectContext HitCtx;
	HitCtx.Instigator = Hero;
	HitCtx.TargetActor = Enemy;
	HitCtx.SkillBaseVo = &NormalAttackVo;

	const float RageBeforeHit = Hero->GetCurrentRage();
	UCombatCalculator::DamagePipeline(Hero, Enemy, HitCtx);
	TestTrue(TEXT("Normal attack DamagePipeline hit adds +0.2 rage"),
		FMath::IsNearlyEqual(Hero->GetCurrentRage(), RageBeforeHit + 0.2f, 1e-3f));

	// 6. HUD 状态区分测试：2.6 怒气 -> 格 0/1 为激活态，格 2 为积累态(0.6)，格 3/4 为空态
	UCombatHUDWidget* Hud = CreateWidget<UCombatHUDWidget>(Scene.World, UCombatHUDWidget::StaticClass());
	TestNotNull(TEXT("UCombatHUDWidget created"), Hud);
	Hero->SetCurrentRage(2.6f);
	Hud->RefreshHUDFromHero(Hero);
	TestEqual(TEXT("HUD reports 2 activated charges"), Hud->GetDisplayedActivatedRageCount(), 2);
	TestTrue(TEXT("HUD reports 0.6 charging progress"), FMath::IsNearlyEqual(Hud->GetDisplayedChargingRageProgress(), 0.6f, 1e-3f));
	TestTrue(TEXT("Pip 0 is activated"), Hud->IsRagePipActivated(0));
	TestTrue(TEXT("Pip 1 is activated"), Hud->IsRagePipActivated(1));
	TestFalse(TEXT("Pip 2 is NOT activated yet"), Hud->IsRagePipActivated(2));
	TestTrue(TEXT("Pip 2 is charging"), Hud->IsRagePipCharging(2));
	TestFalse(TEXT("Pip 3 is neither activated nor charging"), Hud->IsRagePipActivated(3) || Hud->IsRagePipCharging(3));

	return true;
}
#endif
