#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LootWorldSubsystem.generated.h"

class ALootPickupActor;
class FLootPickupInputProcessor;

/**
 * 场景掉落物管理子系统：
 * 1. 监听 F 键（不侵入项目 Enhanced Input），一键拾取玩家附近最近的地面装备。
 * 2. 提供控制台命令 Loot.SpawnHere <ProfileId> [Level=35] [MF=0] [BaseGold=1000] 方便 PIE 随时测试爆装表现。
 */
UCLASS()
class BASIC_TPS_API ULootWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	static ULootWorldSubsystem* Get(const UObject* WorldContext);

	void RegisterPickup(ALootPickupActor* Pickup);
	void UnregisterPickup(ALootPickupActor* Pickup);

	/** 拾取距离玩家最近的可拾取掉落物（默认半径 260cm） */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	bool TryPickupNearestForPlayer(float MaxDistance = 260.0f);

	/** 供输入预处理调用 */
	bool HandleKeyDown(const FKey& Key, bool bIsRepeat);

private:
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
	void SpawnTestLootAtPlayer(FName ProfileId, int32 MonsterLevel, float MagicFind, int32 BaseGold);

	TArray<TWeakObjectPtr<ALootPickupActor>> ActivePickups;
	TSharedPtr<FLootPickupInputProcessor> InputProcessor;
	TArray<IConsoleObject*> ConsoleCommands;
};
