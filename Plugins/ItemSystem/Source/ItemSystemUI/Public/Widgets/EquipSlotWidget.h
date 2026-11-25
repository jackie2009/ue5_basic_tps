#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemSystemTypes.h"
#include "EquipSlotWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UEquipmentComponent;
class UInventoryInteraction;

/** 一个装备位。WBP 需要控件：Frame(Border)、Icon(Image)，可选 Label(TextBlock)。在 WBP 里设置 EquipSlot。 */
UCLASS(Abstract)
class ITEMSYSTEMUI_API UEquipSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(UEquipmentComponent* InEquipment, UInventoryInteraction* InInteraction);
	void Refresh();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip")
	EEquipSlot EquipSlot = EEquipSlot::Head;

	UPROPERTY(EditAnywhere, Category = "Equip")
	FLinearColor NormalColor = FLinearColor(0.08f, 0.08f, 0.09f, 0.9f);

	UPROPERTY(EditAnywhere, Category = "Equip")
	FLinearColor CanDropColor = FLinearColor(0.2f, 0.7f, 0.3f, 0.6f);

	UPROPERTY(EditAnywhere, Category = "Equip")
	FLinearColor HoverColor = FLinearColor(0.25f, 0.25f, 0.28f, 0.95f);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Frame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Label;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UFUNCTION()
	void HandleEquipmentChanged();

	UPROPERTY(Transient)
	TObjectPtr<UEquipmentComponent> Equipment;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryInteraction> Interaction;

	bool bMouseOver = false;
};
