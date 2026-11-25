#include "Widgets/InventoryGridWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Inventory/InventoryGridComponent.h"
#include "Logic/InventoryInteraction.h"
#include "Widgets/ItemIconWidget.h"

void UInventoryGridWidget::Setup(UInventoryGridComponent* InBag, UInventoryInteraction* InInteraction)
{
	if (Bag)
	{
		Bag->OnInventoryChanged.RemoveDynamic(this, &UInventoryGridWidget::HandleInventoryChanged);
	}
	Bag = InBag;
	Interaction = InInteraction;
	if (Bag)
	{
		Bag->OnInventoryChanged.AddUniqueDynamic(this, &UInventoryGridWidget::HandleInventoryChanged);
	}
	Rebuild();
}

void UInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	Rebuild();
}

void UInventoryGridWidget::NativeDestruct()
{
	if (Bag)
	{
		Bag->OnInventoryChanged.RemoveDynamic(this, &UInventoryGridWidget::HandleInventoryChanged);
	}
	Super::NativeDestruct();
}

void UInventoryGridWidget::HandleInventoryChanged()
{
	Rebuild();
}

void UInventoryGridWidget::Rebuild()
{
	if (!GridCanvas || !Bag || !WidgetTree) return;

	GridCanvas->ClearChildren();
	CellWidgets.Reset();

	const int32 W = Bag->Width, H = Bag->Height;
	if (GridSizeBox)
	{
		GridSizeBox->SetWidthOverride(W * CellSize);
		GridSizeBox->SetHeightOverride(H * CellSize);
	}

	// 格子背景（按数据生成，WBP 化以后也保持代码生成）
	for (int32 i = 0; i < W * H; ++i)
	{
		UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Cell->SetBrushColor(CellColor);
		Cell->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* CS = GridCanvas->AddChildToCanvas(Cell))
		{
			CS->SetPosition(FVector2D((i % W) * CellSize + 1.f, (i / W) * CellSize + 1.f));
			CS->SetSize(FVector2D(CellSize - 2.f, CellSize - 2.f));
		}
		CellWidgets.Add(Cell);
	}

	// 物品
	if (ItemWidgetClass)
	{
		for (const FInventoryEntry& E : Bag->GetEntries())
		{
			UItemIconWidget* ItemWidget = CreateWidget<UItemIconWidget>(this, ItemWidgetClass);
			if (!ItemWidget) continue;
			ItemWidget->SetItem(E.Item);
			ItemWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (UCanvasPanelSlot* CS = GridCanvas->AddChildToCanvas(ItemWidget))
			{
				CS->SetPosition(FVector2D((E.Anchor % W) * CellSize, (E.Anchor / W) * CellSize));
				CS->SetSize(FVector2D(E.Size.X * CellSize, E.Size.Y * CellSize));
				CS->SetZOrder(10);
			}
		}
	}
}

bool UInventoryGridWidget::CursorToCell(const FVector2D& ScreenPos, int32& OutCell, FVector2D& OutLocalCells) const
{
	OutCell = -1;
	if (!GridCanvas || !Bag || CellSize <= 0.f) return false;
	const FGeometry& Geo = GridCanvas->GetCachedGeometry();
	const FVector2D Local = Geo.AbsoluteToLocal(ScreenPos);
	OutLocalCells = Local / CellSize;
	const int32 X = FMath::FloorToInt(OutLocalCells.X), Y = FMath::FloorToInt(OutLocalCells.Y);
	if (X < 0 || Y < 0 || X >= Bag->Width || Y >= Bag->Height) return false;
	OutCell = Y * Bag->Width + X;
	return true;
}

int32 UInventoryGridWidget::ComputeTargetAnchor(const FVector2D& LocalCells, FIntPoint Size) const
{
	// 物品中心对准鼠标（同 Unity AdjustEvenSizedAnchorTowardPointer），再夹到网格内
	int32 X = FMath::FloorToInt(LocalCells.X - Size.X * 0.5f + 0.5f);
	int32 Y = FMath::FloorToInt(LocalCells.Y - Size.Y * 0.5f + 0.5f);
	X = FMath::Clamp(X, 0, FMath::Max(0, Bag->Width - Size.X));
	Y = FMath::Clamp(Y, 0, FMath::Max(0, Bag->Height - Size.Y));
	return Y * Bag->Width + X;
}

void UInventoryGridWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateHoverAndHint();
}

void UInventoryGridWidget::UpdateHoverAndHint()
{
	if (!Bag || !Interaction || CellWidgets.Num() != Bag->Width * Bag->Height) return;

	for (UBorder* Cell : CellWidgets)
	{
		Cell->SetBrushColor(CellColor);
	}

	int32 Cell;
	FVector2D LocalCells;
	const bool bInside = FSlateApplication::IsInitialized() &&
		CursorToCell(FSlateApplication::Get().GetCursorPos(), Cell, LocalCells);

	if (!bInside)
	{
		if (bHovering)
		{
			bHovering = false;
			Interaction->SetHovered(FItemInstance());
		}
		return;
	}
	bHovering = true;

	if (!Interaction->IsCarrying())
	{
		FItemInstance Item;
		int32 Anchor;
		Interaction->SetHovered(Bag->FindItemAtCell(Cell, Item, Anchor) ? Item : FItemInstance());
		return;
	}

	// 携带物品：给将要覆盖的格子上色（蓝=空位，黄=交换，红=不可放）
	const FIntPoint Size = Bag->GetItemSize(Interaction->GetCarried());
	const int32 Target = ComputeTargetAnchor(LocalCells, Size);
	int32 Overlap;
	const EInventoryPlacement Result = Interaction->PreviewBagPlacement(Target, Overlap);
	const FLinearColor Color = Result == EInventoryPlacement::Empty ? FreeColor
		: Result == EInventoryPlacement::Swap ? SwapColor : BlockedColor;
	const int32 TX = Target % Bag->Width, TY = Target / Bag->Width;
	for (int32 DY = 0; DY < Size.Y; ++DY)
	{
		for (int32 DX = 0; DX < Size.X; ++DX)
		{
			const int32 X = TX + DX, Y = TY + DY;
			if (X < Bag->Width && Y < Bag->Height)
			{
				CellWidgets[Y * Bag->Width + X]->SetBrushColor(Color);
			}
		}
	}
	Interaction->SetHovered(FItemInstance());
}

FReply UInventoryGridWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	int32 Cell;
	FVector2D LocalCells;
	if (!Interaction || !Bag || !CursorToCell(InMouseEvent.GetScreenSpacePosition(), Cell, LocalCells))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		Interaction->HandleBagRightClick(Cell);
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const int32 Target = Interaction->IsCarrying()
			? ComputeTargetAnchor(LocalCells, Bag->GetItemSize(Interaction->GetCarried()))
			: Cell;
		Interaction->HandleBagLeftClick(Cell, Target);
	}
	return FReply::Handled();
}
