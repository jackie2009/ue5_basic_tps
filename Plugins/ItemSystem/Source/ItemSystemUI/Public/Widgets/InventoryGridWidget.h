#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGridWidget.generated.h"

class UCanvasPanel;
class USizeBox;
class UBorder;
class UItemIconWidget;
class UInventoryGridComponent;
class UInventoryInteraction;

/**
 * 网格背包视图（对应 Unity BagView 的格子区）。格子和物品按数据动态生成（这部分永远是代码）。
 * WBP 需要控件：GridCanvas(CanvasPanel)，可选 GridSizeBox(SizeBox)。
 */
UCLASS(Abstract)
class ITEMSYSTEMUI_API UInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(UInventoryGridComponent* InBag, UInventoryInteraction* InInteraction);

	/** 每格像素（Unity 原图 136 像素/格，这里按屏幕缩小显示） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	float CellSize = 48.f;

	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<UItemIconWidget> ItemWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor CellColor = FLinearColor(0.08f, 0.08f, 0.09f, 0.9f);

	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor FreeColor = FLinearColor(0.25f, 0.55f, 1.f, 0.55f);

	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor SwapColor = FLinearColor(1.f, 0.9f, 0.15f, 0.6f);

	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor BlockedColor = FLinearColor(1.f, 0.2f, 0.2f, 0.6f);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> GridCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> GridSizeBox;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION()
	void HandleInventoryChanged();

	void Rebuild();
	void UpdateHoverAndHint();
	bool CursorToCell(const FVector2D& ScreenPos, int32& OutCell, FVector2D& OutLocalCells) const;
	int32 ComputeTargetAnchor(const FVector2D& LocalCells, FIntPoint Size) const;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryGridComponent> Bag;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryInteraction> Interaction;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> CellWidgets;

	bool bHovering = false;
};
