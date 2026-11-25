#include "Stats/EquipmentStatsLibrary.h"

#include "Data/ItemDatabaseSubsystem.h"
#include "Equipment/EquipmentComponent.h"

FEquipmentStatSummary UEquipmentStatsLibrary::Summarize(const UEquipmentComponent* Equipment)
{
	FEquipmentStatSummary S;
	const UItemDatabaseSubsystem* DB = Equipment ? UItemDatabaseSubsystem::Get(Equipment) : nullptr;
	if (!DB) return S;

	const TArray<FItemInstance>& Slots = Equipment->GetAllSlots();
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		const FItemBaseRow* Row = Slots[i].IsValid() ? DB->Find(Slots[i].BaseID) : nullptr;
		if (!Row) continue;
		S.Defence += Row->Ac;
		S.Block += Row->Block;
		if (!Row->IsWeapon())
		{
			S.BaseAttack += FIntPoint(Row->Dc, Row->Dc2);
		}
	}

	auto WeaponAttack = [&](EEquipSlot Slot) -> FIntPoint
	{
		const FItemInstance Item = Equipment->GetItem(Slot);
		const FItemBaseRow* Row = Item.IsValid() ? DB->Find(Item.BaseID) : nullptr;
		return Row ? FIntPoint(Row->Dc, Row->Dc2) + S.BaseAttack : FIntPoint::ZeroValue;
	};
	S.OneHandAttack = WeaponAttack(EEquipSlot::MainHand);
	S.PolearmAttack = WeaponAttack(EEquipSlot::MainHand2);
	S.RangedAttack = WeaponAttack(EEquipSlot::MainHand3);
	return S;
}
