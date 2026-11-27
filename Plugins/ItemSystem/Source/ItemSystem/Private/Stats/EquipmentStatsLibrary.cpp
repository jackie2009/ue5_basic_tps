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
		const FItemInstance& Item = Slots[i];
		const FItemBaseRow* Row = Item.IsValid() ? DB->Find(Item.BaseID) : nullptr;
		if (!Row) continue;

		const int32 DefPct = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::DefencePercent));
		const int32 DefFlat = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::Defence));
		S.Defence += FMath::RoundToInt(Row->Ac * (1.0f + DefPct / 100.0f)) + DefFlat;

		const int32 BlockPct = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::BlockPercent));
		const int32 BlockFlat = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::Block));
		S.Block += FMath::RoundToInt(Row->Block * (1.0f + BlockPct / 100.0f)) + BlockFlat;

		S.BonusHpMax += Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::HpMax));
		S.Critical += Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::Critical));

		if (!Row->IsWeapon())
		{
			const int32 AtkLow = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::AttackLow));
			const int32 AtkHigh = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::AttackHigh));
			S.BaseAttack += FIntPoint(Row->Dc + AtkLow, Row->Dc2 + AtkHigh);
		}
	}

	auto WeaponAttack = [&](EEquipSlot Slot) -> FIntPoint
	{
		const FItemInstance Item = Equipment->GetItem(Slot);
		const FItemBaseRow* Row = Item.IsValid() ? DB->Find(Item.BaseID) : nullptr;
		if (!Row) return FIntPoint::ZeroValue;

		const int32 AtkPct = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::AttackPercent));
		const int32 AtkLow = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::AttackLow));
		const int32 AtkHigh = Item.GetDynamicAttr(Row->EquipType, static_cast<int32>(EItemAffixAttr::AttackHigh));
		const int32 ScaledLow = FMath::RoundToInt((Row->Dc + AtkLow) * (1.0f + AtkPct / 100.0f));
		const int32 ScaledHigh = FMath::RoundToInt((Row->Dc2 + AtkHigh) * (1.0f + AtkPct / 100.0f));
		return FIntPoint(ScaledLow, ScaledHigh) + S.BaseAttack;
	};
	S.OneHandAttack = WeaponAttack(EEquipSlot::MainHand);
	S.PolearmAttack = WeaponAttack(EEquipSlot::MainHand2);
	S.RangedAttack = WeaponAttack(EEquipSlot::MainHand3);
	return S;
}
