#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/ItemSystemTypes.h"
#include "Inventory/InventoryGridMath.h"
#include "InventoryInteraction.generated.h"

class UInventoryGridComponent;
class UEquipmentComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHoveredItemChanged, const FItemInstance&);

UENUM()
enum class ECarrySource : uint8
{
	None,
	Bag,
	Equipment
};

/**
 * 背包/装备栏交互（对应 Unity PropItemDrag + InventoryMoveTransaction 的核心）：
 * 左键点击拿起物品，物品跟随鼠标，再左键点击放下 / 交换 / 穿戴；右键快速穿戴或卸下。
 * 纯逻辑，不含任何控件；将来换成 WBP 也原样保留。
 */
UCLASS()
class ITEMSYSTEMUI_API UInventoryInteraction : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UInventoryGridComponent* InBag, UEquipmentComponent* InEquipment);

	bool IsCarrying() const { return Carried.IsValid(); }
	const FItemInstance& GetCarried() const { return Carried; }

	UInventoryGridComponent* GetBag() const { return Bag; }
	UEquipmentComponent* GetEquipment() const { return Equipment; }

	void HandleBagLeftClick(int32 ClickedCell, int32 TargetAnchor);
	void HandleBagRightClick(int32 ClickedCell);
	void HandleEquipLeftClick(EEquipSlot Slot);
	void HandleEquipRightClick(EEquipSlot Slot);

	/** 关闭界面时把手上的物品放回原处（放不回就放进背包空位） */
	void CancelCarry();

	EInventoryPlacement PreviewBagPlacement(int32 TargetAnchor, int32& OutOverlapAnchor) const;
	bool CanCarriedEquipTo(EEquipSlot Slot) const;

	void SetHovered(const FItemInstance& Item);
	const FItemInstance& GetHovered() const { return Hovered; }

	FSimpleMulticastDelegate OnCarryChanged;
	FOnHoveredItemChanged OnHoveredChanged;

private:
	void SetCarried(const FItemInstance& Item, ECarrySource InSource, int32 InAnchor, EEquipSlot InSlot);
	void ClearCarried();
	bool PutIntoBag(const FItemInstance& Item, int32 PreferredAnchor);

	UPROPERTY()
	TObjectPtr<UInventoryGridComponent> Bag;

	UPROPERTY()
	TObjectPtr<UEquipmentComponent> Equipment;

	FItemInstance Carried;
	ECarrySource Source = ECarrySource::None;
	int32 SourceAnchor = -1;
	EEquipSlot SourceSlot = EEquipSlot::None;
	FItemInstance Hovered;
};
