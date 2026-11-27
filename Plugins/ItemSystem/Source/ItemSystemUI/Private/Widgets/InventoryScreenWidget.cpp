#include "Widgets/InventoryScreenWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Data/ItemDatabaseSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Inventory/InventoryGridComponent.h"
#include "Logic/InventoryInteraction.h"
#include "Widgets/BagPanelWidget.h"
#include "Widgets/EquipPanelWidget.h"
#include "Widgets/InventoryGridWidget.h"
#include "Widgets/ItemIconWidget.h"

void UInventoryScreenWidget::Setup(UInventoryGridComponent* InBag, UEquipmentComponent* InEquipment)
{
	Bag = InBag;
	Equipment = InEquipment;
	if (!Interaction)
	{
		Interaction = NewObject<UInventoryInteraction>(this);
		Interaction->OnCarryChanged.AddUObject(this, &UInventoryScreenWidget::HandleCarryChanged);
		Interaction->OnHoveredChanged.AddUObject(this, &UInventoryScreenWidget::HandleHoveredChanged);
	}
	Interaction->Initialize(Bag, Equipment);
	Apply();
}

void UInventoryScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible); // 遮住游戏画面的点击，避免误触发攻击
	Apply();
}

void UInventoryScreenWidget::NativeDestruct()
{
	if (Interaction)
	{
		Interaction->CancelCarry();
	}
	Super::NativeDestruct();
}

void UInventoryScreenWidget::Apply()
{
	if (!Interaction) return;
	if (EquipPanel) EquipPanel->Setup(Equipment, Interaction);
	if (BagPanel) BagPanel->Setup(Bag, Interaction);
	HandleCarryChanged();
	HandleHoveredChanged(Interaction->GetHovered());
}

float UInventoryScreenWidget::GetCellSize() const
{
	return (BagPanel && BagPanel->Grid) ? BagPanel->Grid->CellSize : 48.f;
}

void UInventoryScreenWidget::HandleCarryChanged()
{
	if (!CarryIcon || !Interaction) return;
	const bool bCarrying = Interaction->IsCarrying();
	CarryIcon->SetItem(Interaction->GetCarried());
	CarryIcon->SetVisibility(bCarrying ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bCarrying && Bag)
	{
		const FIntPoint Size = Bag->GetItemSize(Interaction->GetCarried());
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(CarryIcon->Slot))
		{
			CS->SetSize(FVector2D(Size.X, Size.Y) * GetCellSize());
		}
	}
}

void UInventoryScreenWidget::HandleHoveredChanged(const FItemInstance& Item)
{
	if (!ItemInfoText) return;
	const UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(this);
	const FItemBaseRow* Row = (DB && Item.IsValid()) ? DB->Find(Item.BaseID) : nullptr;
	if (!Row)
	{
		ItemInfoText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)));
		ItemInfoText->SetText(FText::FromString(TEXT("左键：拿起 / 放下 / 交换    右键：快速穿戴 / 卸下    F：拾取地面掉落    I / Esc：关闭")));
		return;
	}

	ItemInfoText->SetColorAndOpacity(FSlateColor(UItemDatabaseSubsystem::GetQualityColor(Item.Quality)));
	ItemInfoText->SetText(FText::FromString(DB->FormatItemTooltip(Item)));
}

void UInventoryScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!CarryIcon || !Interaction || !Interaction->IsCarrying() || !FSlateApplication::IsInitialized()) return;

	// 跟随鼠标，物品中心对准指针
	if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(CarryIcon->Slot))
	{
		const FVector2D Local = MyGeometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos());
		CS->SetPosition(Local - CS->GetSize() * 0.5f);
	}
}

FReply UInventoryScreenWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 吞掉面板外的点击，不让它传到游戏（攻击等）
	return FReply::Handled();
}
