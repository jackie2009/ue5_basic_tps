#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BagPanelWidget.generated.h"

class UTextBlock;
class UInventoryGridWidget;
class UInventoryGridComponent;
class UInventoryInteraction;

/** 背包面板（对应 Unity BagPanel）。WBP 需要控件：Grid(InventoryGridWidget)，可选 TitleText(TextBlock)。 */
UCLASS(Abstract)
class ITEMSYSTEMUI_API UBagPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(UInventoryGridComponent* InBag, UInventoryInteraction* InInteraction);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> Grid;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

protected:
	virtual void NativeConstruct() override;
	void Apply();

	UPROPERTY(Transient)
	TObjectPtr<UInventoryGridComponent> Bag;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryInteraction> Interaction;
};
