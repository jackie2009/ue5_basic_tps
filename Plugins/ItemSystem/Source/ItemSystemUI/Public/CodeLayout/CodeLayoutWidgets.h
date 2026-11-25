#pragma once

// ============================================================================
// 【临时】代码搭建的布局 —— 将来在 Windows + MCP 环境改为 WBP 后，整个 CodeLayout 目录删除。
// 规则：这里只负责“创建控件并赋值给基类的 BindWidget 成员”，不写任何逻辑。
// 转换方法：为每个基类（Widgets/ 下）新建 WBP，父类选基类，控件名与 BindWidget 成员名一致即可。
// 对照表见同目录 README.md。
// ============================================================================

#include "CoreMinimal.h"
#include "Widgets/ItemIconWidget.h"
#include "Widgets/InventoryGridWidget.h"
#include "Widgets/BagPanelWidget.h"
#include "Widgets/EquipSlotWidget.h"
#include "Widgets/EquipPanelWidget.h"
#include "Widgets/InventoryScreenWidget.h"
#include "CodeLayoutWidgets.generated.h"

/** → 将来 WBP_ItemIcon（父类 ItemIconWidget） */
UCLASS()
class ITEMSYSTEMUI_API UItemIconWidget_Code : public UItemIconWidget
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** → 将来 WBP_InventoryGrid（父类 InventoryGridWidget） */
UCLASS()
class ITEMSYSTEMUI_API UInventoryGridWidget_Code : public UInventoryGridWidget
{
	GENERATED_BODY()
public:
	UInventoryGridWidget_Code(const FObjectInitializer& ObjectInitializer);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** → 将来 WBP_BagPanel（父类 BagPanelWidget） */
UCLASS()
class ITEMSYSTEMUI_API UBagPanelWidget_Code : public UBagPanelWidget
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** → 将来 WBP_EquipSlot（父类 EquipSlotWidget） */
UCLASS()
class ITEMSYSTEMUI_API UEquipSlotWidget_Code : public UEquipSlotWidget
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** → 将来 WBP_EquipPanel（父类 EquipPanelWidget） */
UCLASS()
class ITEMSYSTEMUI_API UEquipPanelWidget_Code : public UEquipPanelWidget
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** → 将来 WBP_InventoryScreen（父类 InventoryScreenWidget），在 Project Settings > Item System UI 里指定 */
UCLASS()
class ITEMSYSTEMUI_API UInventoryScreenWidget_Code : public UInventoryScreenWidget
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
