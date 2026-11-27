#include "Widgets/EquipSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/ItemDatabaseSubsystem.h"
#include "Equipment/EquipmentComponent.h"
#include "Logic/InventoryInteraction.h"

void UEquipSlotWidget::Setup(UEquipmentComponent* InEquipment, UInventoryInteraction* InInteraction)
{
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.RemoveDynamic(this, &UEquipSlotWidget::HandleEquipmentChanged);
	}
	Equipment = InEquipment;
	Interaction = InInteraction;
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.AddUniqueDynamic(this, &UEquipSlotWidget::HandleEquipmentChanged);
	}
	Refresh();
}

void UEquipSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	Refresh();
}

void UEquipSlotWidget::NativeDestruct()
{
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.RemoveDynamic(this, &UEquipSlotWidget::HandleEquipmentChanged);
	}
	Super::NativeDestruct();
}

void UEquipSlotWidget::HandleEquipmentChanged()
{
	Refresh();
}

void UEquipSlotWidget::Refresh()
{
	const FItemInstance Item = Equipment ? Equipment->GetItem(EquipSlot) : FItemInstance();
	if (Icon)
	{
		UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(this);
		UTexture2D* Tex = (DB && Item.IsValid()) ? DB->GetIcon(Item.BaseID) : nullptr;
		Icon->SetBrushFromTexture(Tex, false);
		Icon->SetRenderOpacity(Tex ? 1.f : 0.f);
	}
	if (Label)
	{
		Label->SetText(StaticEnum<EEquipSlot>()->GetDisplayNameTextByValue(static_cast<int64>(EquipSlot)));
		Label->SetVisibility(Item.IsValid() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UEquipSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!Frame) return;
	FLinearColor Color = NormalColor;
	if (Interaction && Interaction->CanCarriedEquipTo(EquipSlot))
	{
		Color = CanDropColor;
	}
	else if (bMouseOver)
	{
		Color = HoverColor;
	}
	else if (Equipment)
	{
		const FItemInstance Equipped = Equipment->GetItem(EquipSlot);
		if (Equipped.IsValid() && Equipped.Quality > 0)
		{
			Color = UItemDatabaseSubsystem::GetQualityColor(Equipped.Quality);
			Color.A = 0.35f;
		}
	}
	Frame->SetBrushColor(Color);
}

FReply UEquipSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!Interaction) return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		Interaction->HandleEquipRightClick(EquipSlot);
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Interaction->HandleEquipLeftClick(EquipSlot);
	}
	return FReply::Handled();
}

void UEquipSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bMouseOver = true;
	if (Interaction && Equipment && !Interaction->IsCarrying())
	{
		Interaction->SetHovered(Equipment->GetItem(EquipSlot));
	}
}

void UEquipSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bMouseOver = false;
	if (Interaction)
	{
		Interaction->SetHovered(FItemInstance());
	}
}
