#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemSystemTypes.h"
#include "InventoryScreenWidget.generated.h"

class UTextBlock;
class UEquipPanelWidget;
class UBagPanelWidget;
class UItemIconWidget;
class UInventoryGridComponent;
class UEquipmentComponent;
class UInventoryInteraction;

/**
 * 背包 + 装备的整屏界面。负责创建交互对象、显示跟随鼠标的物品和物品信息。
 * WBP 需要控件：EquipPanel、BagPanel、CarryIcon（需放在 CanvasPanel 里）、ItemInfoText。
 */
UCLASS(Abstract)
class ITEMSYSTEMUI_API UInventoryScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(UInventoryGridComponent* InBag, UEquipmentComponent* InEquipment);

	UInventoryInteraction* GetInteraction() const { return Interaction; }

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipPanelWidget> EquipPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBagPanelWidget> BagPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemIconWidget> CarryIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemInfoText;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void Apply();
	void HandleCarryChanged();
	void HandleHoveredChanged(const FItemInstance& Item);
	float GetCellSize() const;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryGridComponent> Bag;

	UPROPERTY(Transient)
	TObjectPtr<UEquipmentComponent> Equipment;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryInteraction> Interaction;
};
