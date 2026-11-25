#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "InputCoreTypes.h"
#include "ItemSystemUISubsystem.generated.h"

class APlayerController;
class UInventoryScreenWidget;
class UInventoryGridComponent;
class UEquipmentComponent;
class FItemSystemInputProcessor;

/**
 * 道具系统 UI 入口：按键开关背包界面；首次打开时给 PlayerController 挂上背包/装备组件。
 * 自成一体，不需要修改角色或战斗代码。
 */
UCLASS()
class ITEMSYSTEMUI_API UItemSystemUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	void OpenInventory();

	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	void CloseInventory();

	UFUNCTION(BlueprintPure, Category = "ItemSystem")
	bool IsInventoryOpen() const;

	/** 输入预处理器回调；返回 true 表示消费该按键 */
	bool HandleKeyDown(const FKey& Key, bool bIsRepeat);

private:
	APlayerController* GetPlayerController() const;
	bool IsGameViewportFocused() const;
	void EnsureComponents(APlayerController* PC, UInventoryGridComponent*& OutBag, UEquipmentComponent*& OutEquipment);
	void SeedTestItems(UInventoryGridComponent* Bag, UEquipmentComponent* Equipment);

	UPROPERTY(Transient)
	TObjectPtr<UInventoryScreenWidget> Screen;

	/** [TempRecording] 录像用临时对象，删除时一并去掉 */
	UPROPERTY(Transient)
	TObjectPtr<UObject> TempWeaponSwap;

	TSharedPtr<FItemSystemInputProcessor> InputProcessor;
	bool bPrevShowCursor = false;
};
