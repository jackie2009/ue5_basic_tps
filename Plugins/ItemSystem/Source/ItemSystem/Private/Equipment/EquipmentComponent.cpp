#include "Equipment/EquipmentComponent.h"

#include "Data/ItemDatabaseSubsystem.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.SetNum(static_cast<int32>(EEquipSlot::MAX));
}

bool UEquipmentComponent::CanEquipToSlot(int32 EquipType, EEquipSlot Slot)
{
	if (EquipType <= 0 || Slot == EEquipSlot::None || Slot == EEquipSlot::MAX) return false;
	if (EquipType == static_cast<int32>(EEquipSlot::Finger1))
	{
		return Slot == EEquipSlot::Finger1 || Slot == EEquipSlot::Finger2;
	}
	return EquipType == static_cast<int32>(Slot);
}

int32 UEquipmentComponent::GetEquipType(const FItemInstance& Item) const
{
	const UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(this);
	const FItemBaseRow* Row = DB ? DB->Find(Item.BaseID) : nullptr;
	return Row ? Row->EquipType : 0;
}

EEquipSlot UEquipmentComponent::ResolveSlot(const FItemInstance& Item) const
{
	const int32 Type = GetEquipType(Item);
	if (Type <= 0 || Type >= static_cast<int32>(EEquipSlot::MAX)) return EEquipSlot::None;
	if (Type == static_cast<int32>(EEquipSlot::Finger1))
	{
		if (!GetItem(EEquipSlot::Finger1).IsValid()) return EEquipSlot::Finger1;
		if (!GetItem(EEquipSlot::Finger2).IsValid()) return EEquipSlot::Finger2;
		return EEquipSlot::Finger1;
	}
	return static_cast<EEquipSlot>(Type);
}

bool UEquipmentComponent::Equip(const FItemInstance& Item, EEquipSlot Slot, FItemInstance& OutReplaced)
{
	OutReplaced.Reset();
	if (!Item.IsValid() || !CanEquipToSlot(GetEquipType(Item), Slot)) return false;
	const int32 Index = static_cast<int32>(Slot);
	OutReplaced = Slots[Index];
	Slots[Index] = Item;
	OnEquipmentChanged.Broadcast();
	return true;
}

bool UEquipmentComponent::Unequip(EEquipSlot Slot, FItemInstance& OutItem)
{
	OutItem.Reset();
	const int32 Index = static_cast<int32>(Slot);
	if (!Slots.IsValidIndex(Index) || !Slots[Index].IsValid()) return false;
	OutItem = Slots[Index];
	Slots[Index].Reset();
	OnEquipmentChanged.Broadcast();
	return true;
}

FItemInstance UEquipmentComponent::GetItem(EEquipSlot Slot) const
{
	const int32 Index = static_cast<int32>(Slot);
	return Slots.IsValidIndex(Index) ? Slots[Index] : FItemInstance();
}
