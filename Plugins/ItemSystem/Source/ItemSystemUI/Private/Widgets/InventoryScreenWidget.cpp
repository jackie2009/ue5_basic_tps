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
		ItemInfoText->SetText(FText::FromString(TEXT("左键：拿起 / 放下 / 交换    右键：快速穿戴 / 卸下    I / Esc：关闭")));
		return;
	}

	const FText SlotName = Row->EquipType == static_cast<int32>(EEquipSlot::Finger1)
		? FText::FromString(TEXT("戒指"))
		: StaticEnum<EEquipSlot>()->GetDisplayNameTextByValue(Row->EquipType);
	FString Info = FString::Printf(TEXT("%s    Lv.%d    %s"), *Row->Name, Row->Level, *SlotName.ToString());
	if (Row->Ac > 0) Info += FString::Printf(TEXT("    防御 %d"), Row->Ac);
	if (Row->Dc2 > 0) Info += FString::Printf(TEXT("    攻击 %d-%d"), Row->Dc, Row->Dc2);
	if (Row->Block > 0) Info += FString::Printf(TEXT("    格挡 %d"), Row->Block);
	Info += FString::Printf(TEXT("    占格 %dx%d"), Row->GetCellSize().X, Row->GetCellSize().Y);
	ItemInfoText->SetText(FText::FromString(Info));
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
