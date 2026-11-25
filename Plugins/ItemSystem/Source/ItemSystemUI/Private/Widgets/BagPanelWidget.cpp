#include "Widgets/BagPanelWidget.h"

#include "Widgets/InventoryGridWidget.h"

void UBagPanelWidget::Setup(UInventoryGridComponent* InBag, UInventoryInteraction* InInteraction)
{
	Bag = InBag;
	Interaction = InInteraction;
	Apply();
}

void UBagPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Apply();
}

void UBagPanelWidget::Apply()
{
	if (Grid && Bag)
	{
		Grid->Setup(Bag, Interaction);
	}
}
