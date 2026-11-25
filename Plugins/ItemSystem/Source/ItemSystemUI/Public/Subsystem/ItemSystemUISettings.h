#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "ItemSystemUISettings.generated.h"

class UInventoryScreenWidget;

/** Project Settings > Plugins > Item System UI */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Item System UI"))
class ITEMSYSTEMUI_API UItemSystemUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/** 背包界面类。为空时使用代码搭建的 InventoryScreenWidget_Code；做好 WBP 后在这里指定。 */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftClassPtr<UInventoryScreenWidget> ScreenWidgetClass;

	/** 打开/关闭背包的按键 */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	FKey ToggleKey = EKeys::I;

	UPROPERTY(Config, EditAnywhere, Category = "UI")
	int32 ZOrder = 50;

	/** 首次打开时放入测试装备（每个部位第一件穿上，其余进背包） */
	UPROPERTY(Config, EditAnywhere, Category = "Test")
	bool bSeedTestItems = true;
};
