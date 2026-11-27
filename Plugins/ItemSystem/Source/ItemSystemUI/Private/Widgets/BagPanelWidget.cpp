#include "Widgets/BagPanelWidget.h"

#include "Components/TextBlock.h"
#include "Inventory/InventoryGridComponent.h"
#include "Widgets/InventoryGridWidget.h"

void UBagPanelWidget::Setup(UInventoryGridComponent* InBag, UInventoryInteraction* InInteraction)
{
	if (Bag)
	{
		Bag->OnGoldChanged.RemoveDynamic(this, &UBagPanelWidget::HandleGoldChanged);
	}
	Bag = InBag;
	Interaction = InInteraction;
	if (Bag)
	{
		Bag->OnGoldChanged.AddUniqueDynamic(this, &UBagPanelWidget::HandleGoldChanged);
	}
	Apply();
}

void UBagPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Apply();
}

void UBagPanelWidget::NativeDestruct()
{
	if (Bag)
	{
		Bag->OnGoldChanged.RemoveDynamic(this, &UBagPanelWidget::HandleGoldChanged);
	}
	Super::NativeDestruct();
}

void UBagPanelWidget::Apply()
{
	if (Grid && Bag)
	{
		Grid->Setup(Bag, Interaction);
	}
	RefreshTitle();
}

void UBagPanelWidget::HandleGoldChanged(int32 NewGold, int32 Delta)
{
	RefreshTitle();
}

void UBagPanelWidget::RefreshTitle()
{
	if (!TitleText) return;
	const int32 Gold = Bag ? Bag->Gold : 0;
	TitleText->SetText(FText::FromString(FString::Printf(TEXT("背包    金币: %d"), Gold)));
}
