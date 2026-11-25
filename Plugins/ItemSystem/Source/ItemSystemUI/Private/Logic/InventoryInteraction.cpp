#include "Logic/InventoryInteraction.h"

#include "Equipment/EquipmentComponent.h"
#include "Inventory/InventoryGridComponent.h"

void UInventoryInteraction::Initialize(UInventoryGridComponent* InBag, UEquipmentComponent* InEquipment)
{
	Bag = InBag;
	Equipment = InEquipment;
}

void UInventoryInteraction::SetCarried(const FItemInstance& Item, ECarrySource InSource, int32 InAnchor, EEquipSlot InSlot)
{
	Carried = Item;
	Source = InSource;
	SourceAnchor = InAnchor;
	SourceSlot = InSlot;
	OnCarryChanged.Broadcast();
}

void UInventoryInteraction::ClearCarried()
{
	Carried.Reset();
	Source = ECarrySource::None;
	SourceAnchor = -1;
	SourceSlot = EEquipSlot::None;
	OnCarryChanged.Broadcast();
}

bool UInventoryInteraction::PutIntoBag(const FItemInstance& Item, int32 PreferredAnchor)
{
	if (!Bag || !Item.IsValid()) return false;
	int32 Overlap;
	if (PreferredAnchor >= 0 && Bag->QueryPlace(Item, PreferredAnchor, Overlap) == EInventoryPlacement::Empty)
	{
		FItemInstance Dummy;
		return Bag->PlaceAt(Item, PreferredAnchor, Dummy);
	}
	return Bag->AddItem(Item) >= 0;
}

EInventoryPlacement UInventoryInteraction::PreviewBagPlacement(int32 TargetAnchor, int32& OutOverlapAnchor) const
{
	OutOverlapAnchor = -1;
	return (Bag && IsCarrying()) ? Bag->QueryPlace(Carried, TargetAnchor, OutOverlapAnchor) : EInventoryPlacement::Blocked;
}

bool UInventoryInteraction::CanCarriedEquipTo(EEquipSlot Slot) const
{
	return Equipment && IsCarrying() && UEquipmentComponent::CanEquipToSlot(Equipment->GetEquipType(Carried), Slot);
}

void UInventoryInteraction::HandleBagLeftClick(int32 ClickedCell, int32 TargetAnchor)
{
	if (!Bag) return;
	if (!IsCarrying())
	{
		FItemInstance Item;
		int32 Anchor;
		if (Bag->PickUpAtCell(ClickedCell, Item, Anchor))
		{
			SetCarried(Item, ECarrySource::Bag, Anchor, EEquipSlot::None);
		}
		return;
	}

	FItemInstance Swapped;
	if (Bag->PlaceAt(Carried, TargetAnchor, Swapped))
	{
		if (Swapped.IsValid())
		{
			// 交换：手上换成被压住的那件，它的原位已被占用，取消时放到背包空位
			SetCarried(Swapped, ECarrySource::Bag, -1, EEquipSlot::None);
		}
		else
		{
			ClearCarried();
		}
	}
}

void UInventoryInteraction::HandleBagRightClick(int32 ClickedCell)
{
	if (!Bag || !Equipment || IsCarrying()) return;

	FItemInstance Item;
	int32 Anchor;
	if (!Bag->FindItemAtCell(ClickedCell, Item, Anchor)) return;
	const EEquipSlot Slot = Equipment->ResolveSlot(Item);
	if (Slot == EEquipSlot::None) return;

	Bag->PickUpAtCell(Anchor, Item, Anchor);
	FItemInstance Replaced;
	if (!Equipment->Equip(Item, Slot, Replaced))
	{
		PutIntoBag(Item, Anchor);
		return;
	}
	if (Replaced.IsValid() && !PutIntoBag(Replaced, Anchor))
	{
		// 换下来的装备放不进背包：整体回滚（同 Unity TryPlanEquipmentReplacement）
		FItemInstance Dummy;
		Equipment->Equip(Replaced, Slot, Dummy);
		PutIntoBag(Item, Anchor);
	}
}

void UInventoryInteraction::HandleEquipLeftClick(EEquipSlot Slot)
{
	if (!Equipment) return;
	if (!IsCarrying())
	{
		FItemInstance Item;
		if (Equipment->Unequip(Slot, Item))
		{
			SetCarried(Item, ECarrySource::Equipment, -1, Slot);
		}
		return;
	}

	if (!CanCarriedEquipTo(Slot)) return;
	FItemInstance Replaced;
	if (Equipment->Equip(Carried, Slot, Replaced))
	{
		if (Replaced.IsValid())
		{
			SetCarried(Replaced, ECarrySource::Bag, -1, EEquipSlot::None);
		}
		else
		{
			ClearCarried();
		}
	}
}

void UInventoryInteraction::HandleEquipRightClick(EEquipSlot Slot)
{
	if (!Equipment || !Bag || IsCarrying()) return;
	FItemInstance Item;
	if (!Equipment->Unequip(Slot, Item)) return;
	if (Bag->AddItem(Item) < 0)
	{
		FItemInstance Dummy;
		Equipment->Equip(Item, Slot, Dummy); // 背包满，放回
	}
}

void UInventoryInteraction::CancelCarry()
{
	if (!IsCarrying()) return;

	bool bReturned = false;
	if (Source == ECarrySource::Equipment && Equipment && !Equipment->GetItem(SourceSlot).IsValid())
	{
		FItemInstance Dummy;
		bReturned = Equipment->Equip(Carried, SourceSlot, Dummy);
	}
	if (!bReturned)
	{
		bReturned = PutIntoBag(Carried, Source == ECarrySource::Bag ? SourceAnchor : -1);
	}
	if (bReturned)
	{
		ClearCarried();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSystem: 背包已满，手上的物品无法放回"));
	}
}

void UInventoryInteraction::SetHovered(const FItemInstance& Item)
{
	if (Item.InstanceId == Hovered.InstanceId) return;
	Hovered = Item;
	OnHoveredChanged.Broadcast(Hovered);
}
