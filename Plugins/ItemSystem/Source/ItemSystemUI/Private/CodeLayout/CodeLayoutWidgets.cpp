#include "CodeLayout/CodeLayoutWidgets.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

namespace ItemCodeLayout
{
	const FLinearColor PanelBg(0.02f, 0.02f, 0.025f, 0.92f);
	const FLinearColor TitleColor(0.88f, 0.69f, 0.44f, 1.f);
	const FLinearColor TextColor(0.85f, 0.85f, 0.85f, 1.f);
	constexpr float EquipCell = 40.f;

	UTextBlock* MakeText(UWidgetTree* Tree, FName Name, const FString& Str, int32 Size, const FLinearColor& Color)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		T->SetText(FText::FromString(Str));
		T->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", Size));
		T->SetColorAndOpacity(FSlateColor(Color));
		return T;
	}

	UBorder* MakePanel(UWidgetTree* Tree, FName Name)
	{
		UBorder* B = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		B->SetBrushColor(PanelBg);
		B->SetPadding(FMargin(12.f));
		return B;
	}
}

using namespace ItemCodeLayout;

// ---------------------------------------------------------------- ItemIcon
TSharedRef<SWidget> UItemIconWidget_Code::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Frame"));
		Frame->SetBrushColor(FLinearColor::Transparent);
		Frame->SetPadding(FMargin(2.f));
		Frame->SetHorizontalAlignment(HAlign_Fill);
		Frame->SetVerticalAlignment(VAlign_Fill);
		Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Icon"));
		Frame->SetContent(Icon);
		WidgetTree->RootWidget = Frame;
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- InventoryGrid
UInventoryGridWidget_Code::UInventoryGridWidget_Code(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ItemWidgetClass = UItemIconWidget_Code::StaticClass();
}

TSharedRef<SWidget> UInventoryGridWidget_Code::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		GridSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("GridSizeBox"));
		GridCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("GridCanvas"));
		GridSizeBox->SetContent(GridCanvas);
		WidgetTree->RootWidget = GridSizeBox;
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- BagPanel
TSharedRef<SWidget> UBagPanelWidget_Code::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Root = MakePanel(WidgetTree, TEXT("Background"));
		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VBox"));
		Root->SetContent(VBox);

		TitleText = MakeText(WidgetTree, TEXT("TitleText"), TEXT("背包"), 18, TitleColor);
		if (UVerticalBoxSlot* VS = VBox->AddChildToVerticalBox(TitleText))
		{
			VS->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		}

		Grid = WidgetTree->ConstructWidget<UInventoryGridWidget>(UInventoryGridWidget_Code::StaticClass(), TEXT("Grid"));
		VBox->AddChildToVerticalBox(Grid);

		WidgetTree->RootWidget = Root;
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- EquipSlot
TSharedRef<SWidget> UEquipSlotWidget_Code::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Frame"));
		Frame->SetBrushColor(NormalColor);
		Frame->SetPadding(FMargin(3.f));
		Frame->SetHorizontalAlignment(HAlign_Fill);
		Frame->SetVerticalAlignment(VAlign_Fill);

		UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Overlay"));
		Frame->SetContent(Overlay);

		Label = MakeText(WidgetTree, TEXT("Label"), TEXT(""), 11, FLinearColor(0.5f, 0.5f, 0.5f, 1.f));
		if (UOverlaySlot* OS = Overlay->AddChildToOverlay(Label))
		{
			OS->SetHorizontalAlignment(HAlign_Center);
			OS->SetVerticalAlignment(VAlign_Center);
		}

		Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Icon"));
		Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UOverlaySlot* OS = Overlay->AddChildToOverlay(Icon))
		{
			OS->SetHorizontalAlignment(HAlign_Fill);
			OS->SetVerticalAlignment(VAlign_Fill);
		}

		WidgetTree->RootWidget = Frame;
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- EquipPanel
TSharedRef<SWidget> UEquipPanelWidget_Code::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Root = MakePanel(WidgetTree, TEXT("Background"));
		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VBox"));
		Root->SetContent(VBox);

		if (UVerticalBoxSlot* VS = VBox->AddChildToVerticalBox(MakeText(WidgetTree, TEXT("TitleText"), TEXT("装备"), 18, TitleColor)))
		{
			VS->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		}

		// 纸娃娃布局（以 40 像素为一格）：左列三把武器，中列头/身/腰/脚，右列盾/项链/戒指/手套
		USizeBox* DollBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DollBox"));
		DollBox->SetWidthOverride(280.f);
		DollBox->SetHeightOverride(500.f);
		UCanvasPanel* Doll = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Doll"));
		DollBox->SetContent(Doll);
		VBox->AddChildToVerticalBox(DollBox);

		auto AddSlot = [&](TObjectPtr<UEquipSlotWidget>& Member, const TCHAR* Name, EEquipSlot SlotType, float X, float Y, int32 CW, int32 CH)
		{
			UEquipSlotWidget* W = WidgetTree->ConstructWidget<UEquipSlotWidget>(UEquipSlotWidget_Code::StaticClass(), FName(Name));
			W->EquipSlot = SlotType;
			if (UCanvasPanelSlot* CS = Doll->AddChildToCanvas(W))
			{
				CS->SetPosition(FVector2D(X, Y));
				CS->SetSize(FVector2D(CW * EquipCell, CH * EquipCell));
			}
			Member = W;
		};
		AddSlot(Slot_MainHand, TEXT("Slot_MainHand"), EEquipSlot::MainHand, 0, 0, 2, 4);
		AddSlot(Slot_MainHand2, TEXT("Slot_MainHand2"), EEquipSlot::MainHand2, 0, 170, 2, 4);
		AddSlot(Slot_MainHand3, TEXT("Slot_MainHand3"), EEquipSlot::MainHand3, 0, 340, 2, 4);
		AddSlot(Slot_Head, TEXT("Slot_Head"), EEquipSlot::Head, 100, 0, 2, 2);
		AddSlot(Slot_Body, TEXT("Slot_Body"), EEquipSlot::Body, 100, 90, 2, 3);
		AddSlot(Slot_Belt, TEXT("Slot_Belt"), EEquipSlot::Belt, 100, 220, 2, 1);
		AddSlot(Slot_Feet, TEXT("Slot_Feet"), EEquipSlot::Feet, 100, 270, 2, 2);
		AddSlot(Slot_SubHand, TEXT("Slot_SubHand"), EEquipSlot::SubHand, 200, 0, 2, 4);
		AddSlot(Slot_Neck, TEXT("Slot_Neck"), EEquipSlot::Neck, 220, 170, 1, 1);
		AddSlot(Slot_Finger1, TEXT("Slot_Finger1"), EEquipSlot::Finger1, 200, 220, 1, 1);
		AddSlot(Slot_Finger2, TEXT("Slot_Finger2"), EEquipSlot::Finger2, 240, 220, 1, 1);
		AddSlot(Slot_Wrist, TEXT("Slot_Wrist"), EEquipSlot::Wrist, 200, 270, 2, 2);

		StatsText = MakeText(WidgetTree, TEXT("StatsText"), TEXT(""), 13, TextColor);
		if (UVerticalBoxSlot* VS = VBox->AddChildToVerticalBox(StatsText))
		{
			VS->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
		}

		WidgetTree->RootWidget = Root;
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- InventoryScreen
TSharedRef<SWidget> UInventoryScreenWidget_Code::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));

		UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Panels"));
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(HBox))
		{
			CS->SetAnchors(FAnchors(0.5f, 0.5f));
			CS->SetAlignment(FVector2D(0.5f, 0.5f));
			CS->SetAutoSize(true);
		}

		EquipPanel = WidgetTree->ConstructWidget<UEquipPanelWidget>(UEquipPanelWidget_Code::StaticClass(), TEXT("EquipPanel"));
		if (UHorizontalBoxSlot* HS = HBox->AddChildToHorizontalBox(EquipPanel))
		{
			HS->SetVerticalAlignment(VAlign_Top);
			HS->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));
		}
		BagPanel = WidgetTree->ConstructWidget<UBagPanelWidget>(UBagPanelWidget_Code::StaticClass(), TEXT("BagPanel"));
		if (UHorizontalBoxSlot* HS = HBox->AddChildToHorizontalBox(BagPanel))
		{
			HS->SetVerticalAlignment(VAlign_Top);
		}

		UBorder* InfoBg = MakePanel(WidgetTree, TEXT("ItemInfoBackground"));
		InfoBg->SetVisibility(ESlateVisibility::HitTestInvisible);
		ItemInfoText = MakeText(WidgetTree, TEXT("ItemInfoText"), TEXT(""), 14, TextColor);
		InfoBg->SetContent(ItemInfoText);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(InfoBg))
		{
			CS->SetAnchors(FAnchors(0.5f, 1.f));
			CS->SetAlignment(FVector2D(0.5f, 1.f));
			CS->SetPosition(FVector2D(0.f, -40.f));
			CS->SetAutoSize(true);
		}

		CarryIcon = WidgetTree->ConstructWidget<UItemIconWidget>(UItemIconWidget_Code::StaticClass(), TEXT("CarryIcon"));
		CarryIcon->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(CarryIcon))
		{
			CS->SetZOrder(100);
		}

		WidgetTree->RootWidget = Root;
	}
	return Super::RebuildWidget();
}
