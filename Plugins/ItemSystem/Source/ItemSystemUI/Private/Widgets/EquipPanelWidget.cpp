#include "Widgets/EquipPanelWidget.h"

#include "Components/TextBlock.h"
#include "Equipment/EquipmentComponent.h"
#include "Stats/EquipmentStatsLibrary.h"
#include "Widgets/EquipSlotWidget.h"

TArray<UEquipSlotWidget*> UEquipPanelWidget::GetSlotWidgets() const
{
	return { Slot_Head, Slot_Body, Slot_Wrist, Slot_Feet, Slot_Finger1, Slot_Finger2,
		Slot_MainHand, Slot_Neck, Slot_MainHand2, Slot_MainHand3, Slot_SubHand, Slot_Belt };
}

void UEquipPanelWidget::Setup(UEquipmentComponent* InEquipment, UInventoryInteraction* InInteraction)
{
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.RemoveDynamic(this, &UEquipPanelWidget::RefreshStats);
	}
	Equipment = InEquipment;
	Interaction = InInteraction;
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.AddUniqueDynamic(this, &UEquipPanelWidget::RefreshStats);
	}
	Apply();
}

void UEquipPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Apply();
}

void UEquipPanelWidget::NativeDestruct()
{
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.RemoveDynamic(this, &UEquipPanelWidget::RefreshStats);
	}
	Super::NativeDestruct();
}

void UEquipPanelWidget::Apply()
{
	for (UEquipSlotWidget* W : GetSlotWidgets())
	{
		if (W) W->Setup(Equipment, Interaction);
	}
	RefreshStats();
}

void UEquipPanelWidget::RefreshStats()
{
	if (!StatsText) return;
	const FEquipmentStatSummary S = UEquipmentStatsLibrary::Summarize(Equipment);
	FString Text = FString::Printf(
		TEXT("防御 %d    格挡 %d    生命+%d    暴击+%d\n单手武器 %d-%d\n长柄武器 %d-%d\n远程武器 %d-%d"),
		S.Defence, S.Block, S.BonusHpMax, S.Critical,
		S.OneHandAttack.X, S.OneHandAttack.Y,
		S.PolearmAttack.X, S.PolearmAttack.Y,
		S.RangedAttack.X, S.RangedAttack.Y);
	StatsText->SetText(FText::FromString(Text));
}
