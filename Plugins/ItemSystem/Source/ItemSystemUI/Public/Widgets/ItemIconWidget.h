#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemSystemTypes.h"
#include "ItemIconWidget.generated.h"

class UImage;
class UBorder;

/** 单个道具图标（背包格子里的物品、跟随鼠标的物品）。WBP 需要控件：Icon(Image)，可选 Frame(Border)。 */
UCLASS(Abstract)
class ITEMSYSTEMUI_API UItemIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItem(const FItemInstance& InItem);
	const FItemInstance& GetItem() const { return Item; }

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Frame;

protected:
	virtual void NativeConstruct() override;
	void Apply();

	FItemInstance Item;
};
