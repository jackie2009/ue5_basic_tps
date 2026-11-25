#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipPanelWidget.generated.h"

class UTextBlock;
class UEquipSlotWidget;
class UEquipmentComponent;
class UInventoryInteraction;

/**
 * 装备面板（对应 Unity EquipPanel/EquipView）。
 * WBP 需要控件：12 个 EquipSlotWidget（名字见下，EquipSlot 各自设好）+ StatsText(TextBlock)。
 */
UCLASS(Abstract)
class ITEMSYSTEMUI_API UEquipPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(UEquipmentComponent* InEquipment, UInventoryInteraction* InInteraction);

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Head;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Body;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Wrist;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Feet;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Finger1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Finger2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_MainHand;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Neck;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_MainHand2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_MainHand3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_SubHand;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEquipSlotWidget> Slot_Belt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatsText;

	TArray<UEquipSlotWidget*> GetSlotWidgets() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void RefreshStats();

	void Apply();

	UPROPERTY(Transient)
	TObjectPtr<UEquipmentComponent> Equipment;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryInteraction> Interaction;
};
