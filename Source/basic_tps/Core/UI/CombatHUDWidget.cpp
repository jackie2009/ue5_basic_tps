#include "CombatHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Styling/CoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/HeroCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"

namespace CombatHUDLayout
{
	// 敌人血条（Enemy_Health_Frame: 656x133，Fill 像素区域: x=47..610, y=36..51）
	constexpr float EnemyFrameW = 656.0f;
	constexpr float EnemyFrameH = 133.0f;
	constexpr float EnemyFillX = 47.0f;
	constexpr float EnemyFillY = 36.0f;
	constexpr float EnemyFillW = 563.0f;
	constexpr float EnemyFillH = 15.0f;
	constexpr float EnemyNameX = 110.0f;
	constexpr float EnemyNameY = 58.0f;
	constexpr float EnemyNameW = 436.0f;
	constexpr float EnemyNameH = 38.0f;

	// 右上操作提示（元素图 + 动态文字）
	constexpr float HintsOrnamentW = 34.0f;
	constexpr float HintsOrnamentH = 216.0f;
	constexpr float HintsKeyColumnW = 72.0f;
	constexpr float HintsSmallKeyW = 38.0f;
	constexpr float HintsWideKeyW = 66.0f;
	constexpr float HintsKeyH = 32.0f;

	// 底部总容器：对齐 957px 体力框
	constexpr float BottomClusterW = 957.0f;
	constexpr float BottomClusterH = 148.0f;

	// 玩家血条（Player_Health_Frame: 539x63，Fill 像素区域: x=60..499, y=28..43）
	constexpr float PlayerHealthFrameX = 50.0f;
	constexpr float PlayerHealthFrameY = 4.0f;
	constexpr float PlayerHealthFrameW = 539.0f;
	constexpr float PlayerHealthFrameH = 63.0f;
	constexpr float PlayerHealthFillX = 60.0f;
	constexpr float PlayerHealthFillY = 28.0f;
	constexpr float PlayerHealthFillW = 439.0f;
	constexpr float PlayerHealthFillH = 15.0f;

	// 怒气分割线（Rage_Divider: 29x54）与怒气火焰（Rage_On/Off: 46x62）
	constexpr float RageDividerX = 602.0f;
	constexpr float RageDividerY = 12.0f;
	constexpr float RageDividerW = 29.0f;
	constexpr float RageDividerH = 54.0f;

	constexpr float RageFlamesX = 642.0f;
	constexpr float RageFlamesY = 2.0f;
	constexpr float RageFlameW = 46.0f;
	constexpr float RageFlameH = 58.0f;
	constexpr float RageMiniBarW = 32.0f;
	constexpr float RageMiniBarH = 4.0f;
	constexpr float RageFlamePadX = 3.0f;

	// 体力条（Player_Stamina_Frame: 957x79，Fill 像素区域: x=49..905, y=13..29）
	constexpr float StaminaFrameY = 69.0f;
	constexpr float StaminaFrameW = 957.0f;
	constexpr float StaminaFrameH = 79.0f;
	constexpr float StaminaTroughX = 49.0f;
	constexpr float StaminaTroughY = 12.0f;
	constexpr float StaminaTroughW = 856.0f;
	constexpr float StaminaTroughH = 17.0f;
	constexpr float StaminaFillX = 49.0f;
	constexpr float StaminaFillY = 13.0f;
	constexpr float StaminaFillW = 856.0f;
	constexpr float StaminaFillH = 15.0f;
	constexpr float StaminaDividerW = 2.0f;
}

UCombatHUDWidget::UCombatHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto LoadTex = [](const TCHAR* Path) -> UTexture2D*
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> Finder(Path);
		return Finder.Succeeded() ? Finder.Object : nullptr;
	};
	auto LoadMat = [](const TCHAR* Path) -> UMaterialInterface*
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(Path);
		return Finder.Succeeded() ? Finder.Object : nullptr;
	};

	EnemyHealthFrameTexture    = LoadTex(TEXT("/Game/UI/HUD/Enemy_Health_Frame.Enemy_Health_Frame"));
	EnemyHealthFillMaterial    = LoadMat(TEXT("/Game/UI/HUD/MI_EnemyHealthFill.MI_EnemyHealthFill"));
	ControlHintsBgTexture      = LoadTex(TEXT("/Game/UI/HUD/Control_Hints_Bg.Control_Hints_Bg"));
	ControlHintsOrnamentTexture = LoadTex(TEXT("/Game/UI/HUD/Control_Hints_Ornament.Control_Hints_Ornament"));
	ControlHintsKeycapTexture  = LoadTex(TEXT("/Game/UI/HUD/Control_Hints_Keycap.Control_Hints_Keycap"));
	PlayerHealthFrameTexture   = LoadTex(TEXT("/Game/UI/HUD/Player_Health_Frame.Player_Health_Frame"));
	PlayerHealthFillMaterial   = LoadMat(TEXT("/Game/UI/HUD/MI_PlayerHealthFill.MI_PlayerHealthFill"));
	RageDividerTexture         = LoadTex(TEXT("/Game/UI/HUD/Rage_Divider.Rage_Divider"));
	RageOnTexture              = LoadTex(TEXT("/Game/UI/HUD/Rage_On.Rage_On"));
	RageOffTexture             = LoadTex(TEXT("/Game/UI/HUD/Rage_Off.Rage_Off"));
	RageGlowTexture            = LoadTex(TEXT("/Game/UI/HUD/Rage_Glow.Rage_Glow"));
	PlayerStaminaFrameTexture  = LoadTex(TEXT("/Game/UI/HUD/Player_Stamina_Frame.Player_Stamina_Frame"));
	StaminaExhaustedTexture    = LoadTex(TEXT("/Game/UI/HUD/Stamina_Exhausted.Stamina_Exhausted"));
	StaminaFatiguedTexture     = LoadTex(TEXT("/Game/UI/HUD/Stamina_Fatigued.Stamina_Fatigued"));
	StaminaNormalTexture       = LoadTex(TEXT("/Game/UI/HUD/Stamina_Normal.Stamina_Normal"));
	StaminaVigorousTexture     = LoadTex(TEXT("/Game/UI/HUD/Stamina_Vigorous.Stamina_Vigorous"));

	EnemyNameFont = FCoreStyle::GetDefaultFontStyle("Regular", 22);
	HintKeyFont   = FCoreStyle::GetDefaultFontStyle("Bold", 14);
	HintLabelFont = FCoreStyle::GetDefaultFontStyle("Regular", 18);
}

TSharedRef<SWidget> UCombatHUDWidget::RebuildWidget()
{
	EnsureHudLayoutBuilt();
	return Super::RebuildWidget();
}

void UCombatHUDWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	EnsureHudLayoutBuilt();

	if (IsDesignTime())
	{
		ApplyDesignTimePreview();
	}
	else if (AHeroCharacter* Hero = ResolveHeroCharacter())
	{
		RefreshHUDFromHero(Hero);
	}
}

void UCombatHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureHudLayoutBuilt();

	if (AHeroCharacter* Hero = ResolveHeroCharacter())
	{
		RefreshHUDFromHero(Hero);
	}
}

void UCombatHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (AHeroCharacter* Hero = ResolveHeroCharacter())
	{
		RefreshHUDFromHero(Hero);
	}
	UpdateRageActivationFlash(InDeltaTime);
}

void UCombatHUDWidget::EnsureHudLayoutBuilt()
{
	if (!WidgetTree)
	{
		return;
	}

	if (IsValid(HudRootCanvas) && HudRootCanvas->GetOuter() == WidgetTree)
	{
		return;
	}

	// 清空旧引用，防止重建残留
	StaminaZoneOverlays.Reset();
	StaminaZoneSlots.Reset();
	StaminaZoneBars.Reset();
	StaminaZoneDividers.Reset();
	RageFlameImages.Reset();
	RageGlowImages.Reset();
	RageChargingFillBars.Reset();
	RageMiniGaugeBars.Reset();
	RageFlameOverlays.Reset();
	RageActivationFlashTimers.Reset();
	ControlHintRows.Reset();
	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		LastAppliedZoneCapacities[i] = -1;
	}
	LastAppliedMaxRage = -1;
	LastAppliedFullRage = -1;
	LastAppliedChargingProgress = -1.0f;
	PreviousActivatedRageForFlash = -1;

	// 替换旧原型控件树（保留蓝图层的 character 变量与 EventGraph，不显示旧测试条）
	if (WidgetTree->RootWidget && WidgetTree->RootWidget != HudRootCanvas)
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	HudRootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUD_OverlayCanvas"));
	WidgetTree->RootWidget = HudRootCanvas;
	HudRootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	BuildEnemyHealthSection(HudRootCanvas);
	BuildControlHintsSection(HudRootCanvas);
	BuildPlayerStatusSection(HudRootCanvas);
}

void UCombatHUDWidget::BuildEnemyHealthSection(UCanvasPanel* Root)
{
	EnemyBarContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUD_EnemyBarContainer"));
	if (UCanvasPanelSlot* ChildSlot = Root->AddChildToCanvas(EnemyBarContainer))
	{
		ChildSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
		ChildSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		ChildSlot->SetPosition(FVector2D(0.0f, 18.0f));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::EnemyFrameW, CombatHUDLayout::EnemyFrameH));
	}
	EnemyBarContainer->SetVisibility(ESlateVisibility::Collapsed);

	// 边框底图
	EnemyHealthFrameImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HUD_EnemyHealthFrame"));
	if (EnemyHealthFrameTexture)
	{
		EnemyHealthFrameImage->SetBrushFromTexture(EnemyHealthFrameTexture, false);
	}
	if (UCanvasPanelSlot* ChildSlot = EnemyBarContainer->AddChildToCanvas(EnemyHealthFrameImage))
	{
		ChildSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		ChildSlot->SetOffsets(FMargin(0.0f));
		ChildSlot->SetZOrder(0);
	}

	// 红色血量填充条（直接使用 MI_EnemyHealthFill）
	EnemyHealthFillBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HUD_EnemyHealthFill"));
	{
		FProgressBarStyle BarStyle;
		BarStyle.BackgroundImage.DrawAs = ESlateBrushDrawType::NoDrawType;
		BarStyle.FillImage.DrawAs = ESlateBrushDrawType::Image;
		BarStyle.FillImage.SetResourceObject(EnemyHealthFillMaterial);
		BarStyle.FillImage.ImageSize = FVector2D(CombatHUDLayout::EnemyFillW, CombatHUDLayout::EnemyFillH);
		EnemyHealthFillBar->SetWidgetStyle(BarStyle);
		EnemyHealthFillBar->SetFillColorAndOpacity(FLinearColor::White);
		EnemyHealthFillBar->SetBarFillType(EProgressBarFillType::LeftToRight);
		EnemyHealthFillBar->SetPercent(1.0f);
	}
	if (UCanvasPanelSlot* ChildSlot = EnemyBarContainer->AddChildToCanvas(EnemyHealthFillBar))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::EnemyFillX, CombatHUDLayout::EnemyFillY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::EnemyFillW, CombatHUDLayout::EnemyFillH));
		ChildSlot->SetZOrder(1);
	}

	// 敌人名字
	EnemyNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HUD_EnemyNameText"));
	EnemyNameText->SetFont(EnemyNameFont);
	EnemyNameText->SetJustification(ETextJustify::Center);
	EnemyNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.90f, 0.78f, 1.0f)));
	EnemyNameText->SetShadowOffset(FVector2D(1.5f, 1.5f));
	EnemyNameText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
	EnemyNameText->SetText(FText::GetEmpty());
	if (UCanvasPanelSlot* ChildSlot = EnemyBarContainer->AddChildToCanvas(EnemyNameText))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::EnemyNameX, CombatHUDLayout::EnemyNameY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::EnemyNameW, CombatHUDLayout::EnemyNameH));
		ChildSlot->SetZOrder(2);
	}
}

void UCombatHUDWidget::BuildControlHintsSection(UCanvasPanel* Root)
{
	ControlHintsContainer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("HUD_ControlHintsContainer"));
	if (UCanvasPanelSlot* ChildSlot = Root->AddChildToCanvas(ControlHintsContainer))
	{
		ChildSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
		ChildSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		ChildSlot->SetPosition(FVector2D(-28.0f, 72.0f));
		ChildSlot->SetAutoSize(true);
	}
	ControlHintsContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 1. 柔和暗色羽化背景元素图
	ControlHintsBgImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HUD_ControlHintsBg"));
	if (ControlHintsBgTexture)
	{
		ControlHintsBgImage->SetBrushFromTexture(ControlHintsBgTexture, false);
	}
	else
	{
		ControlHintsBgImage->SetColorAndOpacity(FLinearColor(0.04f, 0.04f, 0.04f, 0.65f));
	}
	if (UOverlaySlot* BgSlot = ControlHintsContainer->AddChildToOverlay(ControlHintsBgImage))
	{
		BgSlot->SetHorizontalAlignment(HAlign_Fill);
		BgSlot->SetVerticalAlignment(VAlign_Fill);
		BgSlot->SetPadding(FMargin(18.0f, 0.0f, 0.0f, 0.0f));
	}

	// 2. 左侧金色菱形竖线装饰 + 右侧四行动态按键提示
	UHorizontalBox* OuterRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HUD_ControlHintsOuterRow"));
	if (UOverlaySlot* ContentSlot = ControlHintsContainer->AddChildToOverlay(OuterRow))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Center);
		ContentSlot->SetPadding(FMargin(4.0f, 14.0f, 28.0f, 14.0f));
	}

	USizeBox* OrnamentBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HUD_ControlHintsOrnamentBox"));
	OrnamentBox->SetWidthOverride(CombatHUDLayout::HintsOrnamentW);
	OrnamentBox->SetHeightOverride(CombatHUDLayout::HintsOrnamentH);
	ControlHintsOrnamentImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HUD_ControlHintsOrnament"));
	if (ControlHintsOrnamentTexture)
	{
		ControlHintsOrnamentImage->SetBrushFromTexture(ControlHintsOrnamentTexture, false);
	}
	OrnamentBox->AddChild(ControlHintsOrnamentImage);
	if (UHorizontalBoxSlot* OrnSlot = OuterRow->AddChildToHorizontalBox(OrnamentBox))
	{
		OrnSlot->SetHorizontalAlignment(HAlign_Center);
		OrnSlot->SetVerticalAlignment(VAlign_Center);
		OrnSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
	}

	UVerticalBox* RowsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HUD_ControlHintsRows"));
	if (UHorizontalBoxSlot* RowsSlot = OuterRow->AddChildToHorizontalBox(RowsBox))
	{
		RowsSlot->SetHorizontalAlignment(HAlign_Left);
		RowsSlot->SetVerticalAlignment(VAlign_Center);
	}

	auto AddHintRow = [&](const FName& Suffix, const FText& KeyLabel, float KeycapWidth, const FText& InitialActionText) -> UTextBlock*
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			*FString::Printf(TEXT("HUD_HintRow_%s"), *Suffix.ToString()));

		// 固定宽度的键帽列，确保不同宽度键帽右侧的技能/动作名左对齐
		USizeBox* KeyColBox = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			*FString::Printf(TEXT("HUD_HintKeyCol_%s"), *Suffix.ToString()));
		KeyColBox->SetWidthOverride(CombatHUDLayout::HintsKeyColumnW);
		KeyColBox->SetHeightOverride(CombatHUDLayout::HintsKeyH);

		UOverlay* KeyColAlign = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			*FString::Printf(TEXT("HUD_HintKeyAlign_%s"), *Suffix.ToString()));
		KeyColBox->AddChild(KeyColAlign);

		USizeBox* KeycapSize = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			*FString::Printf(TEXT("HUD_HintKeycapSize_%s"), *Suffix.ToString()));
		KeycapSize->SetWidthOverride(KeycapWidth);
		KeycapSize->SetHeightOverride(CombatHUDLayout::HintsKeyH);

		UBorder* KeycapBorder = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			*FString::Printf(TEXT("HUD_HintKeycap_%s"), *Suffix.ToString()));
		if (ControlHintsKeycapTexture)
		{
			FSlateBrush KeyBrush;
			KeyBrush.DrawAs = ESlateBrushDrawType::Box;
			KeyBrush.Margin = FMargin(0.28f);
			KeyBrush.SetResourceObject(ControlHintsKeycapTexture);
			KeyBrush.ImageSize = FVector2D(ControlHintsKeycapTexture->GetSizeX(), ControlHintsKeycapTexture->GetSizeY());
			KeycapBorder->SetBrush(KeyBrush);
			KeycapBorder->SetBrushColor(FLinearColor::White);
		}
		else
		{
			KeycapBorder->SetBrushColor(FLinearColor(0.08f, 0.07f, 0.06f, 0.92f));
		}
		KeycapBorder->SetHorizontalAlignment(HAlign_Center);
		KeycapBorder->SetVerticalAlignment(VAlign_Center);
		KeycapBorder->SetPadding(FMargin(4.0f, 1.0f));

		UTextBlock* KeyText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			*FString::Printf(TEXT("HUD_HintKeyText_%s"), *Suffix.ToString()));
		KeyText->SetFont(HintKeyFont);
		KeyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.90f, 0.78f, 1.0f)));
		KeyText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		KeyText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
		KeyText->SetJustification(ETextJustify::Center);
		KeyText->SetText(KeyLabel);
		KeycapBorder->SetContent(KeyText);
		KeycapSize->AddChild(KeycapBorder);

		if (UOverlaySlot* CapSlot = KeyColAlign->AddChildToOverlay(KeycapSize))
		{
			CapSlot->SetHorizontalAlignment(HAlign_Left);
			CapSlot->SetVerticalAlignment(VAlign_Center);
		}

		if (UHorizontalBoxSlot* KeyColSlot = Row->AddChildToHorizontalBox(KeyColBox))
		{
			KeyColSlot->SetHorizontalAlignment(HAlign_Left);
			KeyColSlot->SetVerticalAlignment(VAlign_Center);
			KeyColSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
		}

		UTextBlock* ActionText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			*FString::Printf(TEXT("HUD_HintLabel_%s"), *Suffix.ToString()));
		ActionText->SetFont(HintLabelFont);
		ActionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.89f, 0.76f, 1.0f)));
		ActionText->SetShadowOffset(FVector2D(1.5f, 1.5f));
		ActionText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
		ActionText->SetText(InitialActionText);
		if (UHorizontalBoxSlot* ActionSlot = Row->AddChildToHorizontalBox(ActionText))
		{
			ActionSlot->SetHorizontalAlignment(HAlign_Left);
			ActionSlot->SetVerticalAlignment(VAlign_Center);
		}

		if (UVerticalBoxSlot* VRowSlot = RowsBox->AddChildToVerticalBox(Row))
		{
			VRowSlot->SetHorizontalAlignment(HAlign_Left);
			VRowSlot->SetVerticalAlignment(VAlign_Center);
			VRowSlot->SetPadding(FMargin(0.0f, 6.0f));
		}

		ControlHintRows.Add(Row);
		return ActionText;
	};

	HintSkillQLabelText = AddHintRow(TEXT("SkillQ"), FText::FromString(TEXT("Q")), CombatHUDLayout::HintsSmallKeyW, FText::FromString(TEXT("技能一")));
	HintSkillELabelText = AddHintRow(TEXT("SkillE"), FText::FromString(TEXT("E")), CombatHUDLayout::HintsSmallKeyW, FText::FromString(TEXT("技能二")));
	HintDodgeLabelText  = AddHintRow(TEXT("Dodge"), FText::FromString(TEXT("Shift")), CombatHUDLayout::HintsWideKeyW, DodgeHintText);
	HintJumpLabelText   = AddHintRow(TEXT("Jump"), JumpKeyText, CombatHUDLayout::HintsWideKeyW, JumpHintText);
}

void UCombatHUDWidget::BuildPlayerStatusSection(UCanvasPanel* Root)
{
	PlayerStatusContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUD_PlayerStatusContainer"));
	if (UCanvasPanelSlot* ChildSlot = Root->AddChildToCanvas(PlayerStatusContainer))
	{
		ChildSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
		ChildSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		ChildSlot->SetPosition(FVector2D(0.0f, -24.0f));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::BottomClusterW, CombatHUDLayout::BottomClusterH));
	}
	PlayerStatusContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 1. 玩家血条容器（539x63）
	UCanvasPanel* PlayerHealthPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUD_PlayerHealthPanel"));
	if (UCanvasPanelSlot* ChildSlot = PlayerStatusContainer->AddChildToCanvas(PlayerHealthPanel))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::PlayerHealthFrameX, CombatHUDLayout::PlayerHealthFrameY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::PlayerHealthFrameW, CombatHUDLayout::PlayerHealthFrameH));
	}

	PlayerHealthFrameImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HUD_PlayerHealthFrame"));
	if (PlayerHealthFrameTexture)
	{
		PlayerHealthFrameImage->SetBrushFromTexture(PlayerHealthFrameTexture, false);
	}
	if (UCanvasPanelSlot* ChildSlot = PlayerHealthPanel->AddChildToCanvas(PlayerHealthFrameImage))
	{
		ChildSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		ChildSlot->SetOffsets(FMargin(0.0f));
		ChildSlot->SetZOrder(0);
	}

	PlayerHealthFillBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HUD_PlayerHealthFill"));
	{
		FProgressBarStyle BarStyle;
		BarStyle.BackgroundImage.DrawAs = ESlateBrushDrawType::NoDrawType;
		BarStyle.FillImage.DrawAs = ESlateBrushDrawType::Image;
		BarStyle.FillImage.SetResourceObject(PlayerHealthFillMaterial);
		BarStyle.FillImage.ImageSize = FVector2D(CombatHUDLayout::PlayerHealthFillW, CombatHUDLayout::PlayerHealthFillH);
		PlayerHealthFillBar->SetWidgetStyle(BarStyle);
		PlayerHealthFillBar->SetFillColorAndOpacity(FLinearColor::White);
		PlayerHealthFillBar->SetBarFillType(EProgressBarFillType::LeftToRight);
		PlayerHealthFillBar->SetPercent(1.0f);
	}
	if (UCanvasPanelSlot* ChildSlot = PlayerHealthPanel->AddChildToCanvas(PlayerHealthFillBar))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::PlayerHealthFillX, CombatHUDLayout::PlayerHealthFillY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::PlayerHealthFillW, CombatHUDLayout::PlayerHealthFillH));
		ChildSlot->SetZOrder(1);
	}

	// 2. 血条与怒气之间的金色竖线分割符
	RageDividerImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HUD_RageDivider"));
	if (RageDividerTexture)
	{
		RageDividerImage->SetBrushFromTexture(RageDividerTexture, false);
	}
	if (UCanvasPanelSlot* ChildSlot = PlayerStatusContainer->AddChildToCanvas(RageDividerImage))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::RageDividerX, CombatHUDLayout::RageDividerY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::RageDividerW, CombatHUDLayout::RageDividerH));
	}

	// 3. 怒气火焰横排容器
	RageFlamesBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HUD_RageFlamesBox"));
	if (UCanvasPanelSlot* ChildSlot = PlayerStatusContainer->AddChildToCanvas(RageFlamesBox))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::RageFlamesX, CombatHUDLayout::RageFlamesY));
		ChildSlot->SetAutoSize(true);
	}
	RebuildRageFlameWidgets(5);

	// 4. 底部四段动态比例体力条（957x79）
	StaminaBarContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUD_StaminaBarContainer"));
	if (UCanvasPanelSlot* ChildSlot = PlayerStatusContainer->AddChildToCanvas(StaminaBarContainer))
	{
		ChildSlot->SetPosition(FVector2D(0.0f, CombatHUDLayout::StaminaFrameY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::StaminaFrameW, CombatHUDLayout::StaminaFrameH));
	}

	StaminaFrameImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HUD_StaminaFrame"));
	if (PlayerStaminaFrameTexture)
	{
		StaminaFrameImage->SetBrushFromTexture(PlayerStaminaFrameTexture, false);
	}
	if (UCanvasPanelSlot* ChildSlot = StaminaBarContainer->AddChildToCanvas(StaminaFrameImage))
	{
		ChildSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		ChildSlot->SetOffsets(FMargin(0.0f));
		ChildSlot->SetZOrder(0);
	}

	// 遮盖 Player_Stamina_Frame 内部静态均分分隔线，使动态比例区域分割线清晰干净
	StaminaTroughBackdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HUD_StaminaTroughBackdrop"));
	StaminaTroughBackdrop->SetBrushColor(FLinearColor(0.05f, 0.045f, 0.04f, 1.0f));
	if (UCanvasPanelSlot* ChildSlot = StaminaBarContainer->AddChildToCanvas(StaminaTroughBackdrop))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::StaminaTroughX, CombatHUDLayout::StaminaTroughY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::StaminaTroughW, CombatHUDLayout::StaminaTroughH));
		ChildSlot->SetZOrder(1);
	}

	StaminaZonesBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HUD_StaminaZonesBox"));
	if (UCanvasPanelSlot* ChildSlot = StaminaBarContainer->AddChildToCanvas(StaminaZonesBox))
	{
		ChildSlot->SetPosition(FVector2D(CombatHUDLayout::StaminaFillX, CombatHUDLayout::StaminaFillY));
		ChildSlot->SetSize(FVector2D(CombatHUDLayout::StaminaFillW, CombatHUDLayout::StaminaFillH));
		ChildSlot->SetZOrder(2);
	}

	UTexture2D* ZoneTextures[HeroStaminaDefaults::ZoneCount] = {
		StaminaExhaustedTexture,
		StaminaFatiguedTexture,
		StaminaNormalTexture,
		StaminaVigorousTexture
	};
	// 各段空槽微暗底色：体力未满时仍能直观辨识四段区间长度比例
	const FLinearColor ZoneTrackTints[HeroStaminaDefaults::ZoneCount] = {
		FLinearColor(0.14f, 0.06f, 0.05f, 0.92f), // 无力区暗红底
		FLinearColor(0.16f, 0.11f, 0.04f, 0.92f), // 疲劳区暗琥珀底
		FLinearColor(0.12f, 0.14f, 0.05f, 0.92f), // 普通区暗橄榄底
		FLinearColor(0.05f, 0.14f, 0.08f, 0.92f)  // 活力区暗翠绿底
	};

	StaminaZoneOverlays.Reserve(HeroStaminaDefaults::ZoneCount);
	StaminaZoneSlots.Reserve(HeroStaminaDefaults::ZoneCount);
	StaminaZoneBars.Reserve(HeroStaminaDefaults::ZoneCount);
	StaminaZoneDividers.Reserve(HeroStaminaDefaults::ZoneCount - 1);

	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		UOverlay* ZoneOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			*FString::Printf(TEXT("HUD_StaminaZoneOverlay_%d"), i));

		UBorder* EmptyTrack = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			*FString::Printf(TEXT("HUD_StaminaZoneTrack_%d"), i));
		EmptyTrack->SetBrushColor(ZoneTrackTints[i]);
		if (UOverlaySlot* TrackSlot = ZoneOverlay->AddChildToOverlay(EmptyTrack))
		{
			TrackSlot->SetHorizontalAlignment(HAlign_Fill);
			TrackSlot->SetVerticalAlignment(VAlign_Fill);
		}

		UProgressBar* ZoneBar = WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(),
			*FString::Printf(TEXT("HUD_StaminaZoneBar_%d"), i));
		{
			FProgressBarStyle ZoneStyle;
			ZoneStyle.BackgroundImage.DrawAs = ESlateBrushDrawType::NoDrawType;
			ZoneStyle.FillImage.DrawAs = ESlateBrushDrawType::Image;
			ZoneStyle.FillImage.SetResourceObject(ZoneTextures[i]);
			if (ZoneTextures[i])
			{
				ZoneStyle.FillImage.ImageSize = FVector2D(ZoneTextures[i]->GetSizeX(), ZoneTextures[i]->GetSizeY());
			}
			ZoneBar->SetWidgetStyle(ZoneStyle);
			ZoneBar->SetFillColorAndOpacity(FLinearColor::White);
			ZoneBar->SetBarFillType(EProgressBarFillType::LeftToRight);
			ZoneBar->SetPercent(1.0f);
		}
		if (UOverlaySlot* BarSlot = ZoneOverlay->AddChildToOverlay(ZoneBar))
		{
			BarSlot->SetHorizontalAlignment(HAlign_Fill);
			BarSlot->SetVerticalAlignment(VAlign_Fill);
		}

		UHorizontalBoxSlot* HSlot = StaminaZonesBox->AddChildToHorizontalBox(ZoneOverlay);
		if (HSlot)
		{
			FSlateChildSize FillSize(ESlateSizeRule::Fill);
			FillSize.Value = 1.0f;
			HSlot->SetSize(FillSize);
			HSlot->SetHorizontalAlignment(HAlign_Fill);
			HSlot->SetVerticalAlignment(VAlign_Fill);
		}

		StaminaZoneOverlays.Add(ZoneOverlay);
		StaminaZoneSlots.Add(HSlot);
		StaminaZoneBars.Add(ZoneBar);

		if (i < HeroStaminaDefaults::ZoneCount - 1)
		{
			UBorder* Divider = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(),
				*FString::Printf(TEXT("HUD_StaminaZoneDivider_%d"), i));
			Divider->SetBrushColor(FLinearColor(0.55f, 0.44f, 0.26f, 0.95f));

			USizeBox* DivSizeBox = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(),
				*FString::Printf(TEXT("HUD_StaminaZoneDividerSize_%d"), i));
			DivSizeBox->SetWidthOverride(CombatHUDLayout::StaminaDividerW);
			Divider->SetContent(DivSizeBox);

			if (UHorizontalBoxSlot* DivSlot = StaminaZonesBox->AddChildToHorizontalBox(Divider))
			{
				DivSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
				DivSlot->SetHorizontalAlignment(HAlign_Center);
				DivSlot->SetVerticalAlignment(VAlign_Fill);
				DivSlot->SetPadding(FMargin(1.0f, 0.0f, 1.0f, 0.0f));
			}

			StaminaZoneDividers.Add(Divider);
		}
	}
}

void UCombatHUDWidget::RebuildRageFlameWidgets(int32 MaxCount)
{
	if (!WidgetTree || !IsValid(RageFlamesBox))
	{
		return;
	}

	const int32 ClampedMax = FMath::Clamp(MaxCount, 1, 10);
	if (ClampedMax == LastAppliedMaxRage && RageFlameImages.Num() == ClampedMax)
	{
		return;
	}

	RageFlamesBox->ClearChildren();
	RageFlameImages.Reset();
	RageGlowImages.Reset();
	RageChargingFillBars.Reset();
	RageMiniGaugeBars.Reset();
	RageFlameOverlays.Reset();
	RageActivationFlashTimers.Init(0.0f, ClampedMax);
	LastAppliedMaxRage = ClampedMax;
	LastAppliedFullRage = -1;
	LastAppliedChargingProgress = -1.0f;

	for (int32 i = 0; i < ClampedMax; ++i)
	{
		UVerticalBox* PipColumn = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			*FString::Printf(TEXT("HUD_RagePipCol_%d"), i));

		// 上半部分：光晕 + 火焰底图 + 自下而上积累填充层
		USizeBox* FlameBox = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			*FString::Printf(TEXT("HUD_RageFlameBox_%d"), i));
		FlameBox->SetWidthOverride(CombatHUDLayout::RageFlameW);
		FlameBox->SetHeightOverride(CombatHUDLayout::RageFlameH);

		UOverlay* FlameOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			*FString::Printf(TEXT("HUD_RageFlameOverlay_%d"), i));
		FlameOverlay->SetRenderTransformPivot(FVector2D(0.5f, 0.55f));
		FlameBox->AddChild(FlameOverlay);

		// Layer 0：激活态专属金色辉光光晕（仅完全激活的可用怒气点显示）
		UImage* GlowImg = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(),
			*FString::Printf(TEXT("HUD_RageGlow_%d"), i));
		if (RageGlowTexture)
		{
			GlowImg->SetBrushFromTexture(RageGlowTexture, false);
		}
		else
		{
			GlowImg->SetColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.18f, 0.55f));
		}
		GlowImg->SetRenderScale(FVector2D(1.38f, 1.32f));
		GlowImg->SetVisibility(ESlateVisibility::Collapsed);
		if (UOverlaySlot* GlowSlot = FlameOverlay->AddChildToOverlay(GlowImg))
		{
			GlowSlot->SetHorizontalAlignment(HAlign_Fill);
			GlowSlot->SetVerticalAlignment(VAlign_Fill);
		}

		// Layer 1：主火焰图标（激活态用明亮 Rage_On；积累/空态用暗色 Rage_Off）
		UImage* FlameImg = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(),
			*FString::Printf(TEXT("HUD_RageFlame_%d"), i));
		if (RageOffTexture)
		{
			FlameImg->SetBrushFromTexture(RageOffTexture, false);
		}
		if (UOverlaySlot* ImgSlot = FlameOverlay->AddChildToOverlay(FlameImg))
		{
			ImgSlot->SetHorizontalAlignment(HAlign_Fill);
			ImgSlot->SetVerticalAlignment(VAlign_Fill);
		}

		// Layer 2：积累过程专用（自下而上暗琥珀余烬填充，一眼区分“未满激活”）
		UProgressBar* ChargeFillBar = WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(),
			*FString::Printf(TEXT("HUD_RageChargeFill_%d"), i));
		{
			FProgressBarStyle ChargeStyle;
			ChargeStyle.BackgroundImage.DrawAs = ESlateBrushDrawType::NoDrawType;
			ChargeStyle.FillImage.DrawAs = ESlateBrushDrawType::Image;
			ChargeStyle.FillImage.SetResourceObject(RageOnTexture);
			if (RageOnTexture)
			{
				ChargeStyle.FillImage.ImageSize = FVector2D(RageOnTexture->GetSizeX(), RageOnTexture->GetSizeY());
			}
			ChargeFillBar->SetWidgetStyle(ChargeStyle);
			// 深铜琥珀色：与完全激活的炽亮金焰形成清晰明度与色相区分
			ChargeFillBar->SetFillColorAndOpacity(FLinearColor(0.62f, 0.28f, 0.09f, 0.85f));
			ChargeFillBar->SetBarFillType(EProgressBarFillType::BottomToTop);
			ChargeFillBar->SetPercent(0.0f);
			ChargeFillBar->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (UOverlaySlot* ChargeSlot = FlameOverlay->AddChildToOverlay(ChargeFillBar))
		{
			ChargeSlot->SetHorizontalAlignment(HAlign_Fill);
			ChargeSlot->SetVerticalAlignment(VAlign_Fill);
		}

		if (UVerticalBoxSlot* FlameRowSlot = PipColumn->AddChildToVerticalBox(FlameBox))
		{
			FlameRowSlot->SetHorizontalAlignment(HAlign_Center);
			FlameRowSlot->SetVerticalAlignment(VAlign_Center);
		}

		// 下半部分：底部细进度槽（已激活=亮金满槽；积累中=暗底琥珀分段进度；空=暗槽）
		USizeBox* GaugeBox = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			*FString::Printf(TEXT("HUD_RageGaugeBox_%d"), i));
		GaugeBox->SetWidthOverride(CombatHUDLayout::RageMiniBarW);
		GaugeBox->SetHeightOverride(CombatHUDLayout::RageMiniBarH);

		UProgressBar* MiniGauge = WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(),
			*FString::Printf(TEXT("HUD_RageMiniGauge_%d"), i));
		{
			FProgressBarStyle GaugeStyle;
			GaugeStyle.BackgroundImage.DrawAs = ESlateBrushDrawType::Box;
			GaugeStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(0.08f, 0.07f, 0.06f, 0.85f));
			GaugeStyle.FillImage.DrawAs = ESlateBrushDrawType::Box;
			GaugeStyle.FillImage.TintColor = FSlateColor(FLinearColor::White);
			MiniGauge->SetWidgetStyle(GaugeStyle);
			MiniGauge->SetBarFillType(EProgressBarFillType::LeftToRight);
			MiniGauge->SetPercent(0.0f);
		}
		GaugeBox->AddChild(MiniGauge);

		if (UVerticalBoxSlot* GaugeRowSlot = PipColumn->AddChildToVerticalBox(GaugeBox))
		{
			GaugeRowSlot->SetHorizontalAlignment(HAlign_Center);
			GaugeRowSlot->SetVerticalAlignment(VAlign_Top);
			GaugeRowSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
		}

		if (UHorizontalBoxSlot* FlameSlot = RageFlamesBox->AddChildToHorizontalBox(PipColumn))
		{
			FlameSlot->SetPadding(FMargin(CombatHUDLayout::RageFlamePadX, 0.0f));
			FlameSlot->SetHorizontalAlignment(HAlign_Center);
			FlameSlot->SetVerticalAlignment(VAlign_Center);
		}

		RageFlameOverlays.Add(FlameOverlay);
		RageGlowImages.Add(GlowImg);
		RageFlameImages.Add(FlameImg);
		RageChargingFillBars.Add(ChargeFillBar);
		RageMiniGaugeBars.Add(MiniGauge);
	}
}

void UCombatHUDWidget::UpdateRageFlameDisplay(float CurrentRageValue, int32 MaxCount)
{
	RebuildRageFlameWidgets(MaxCount);

	const int32 PipCount = RageFlameImages.Num();
	const float ClampedRage = FMath::Clamp(CurrentRageValue, 0.0f, static_cast<float>(PipCount));
	const int32 FullCharges = FMath::Clamp(FMath::FloorToInt(ClampedRage + 1e-4f), 0, PipCount);
	const float ChargingFrac = (FullCharges < PipCount) ? FMath::Clamp(ClampedRage - static_cast<float>(FullCharges), 0.0f, 1.0f) : 0.0f;

	// 检测新激活的怒气点，触发点燃弹出动效
	if (PreviousActivatedRageForFlash >= 0 && FullCharges > PreviousActivatedRageForFlash)
	{
		for (int32 idx = PreviousActivatedRageForFlash; idx < FullCharges && idx < RageActivationFlashTimers.Num(); ++idx)
		{
			RageActivationFlashTimers[idx] = 0.35f;
		}
	}
	PreviousActivatedRageForFlash = FullCharges;

	if (FullCharges == LastAppliedFullRage && FMath::IsNearlyEqual(ChargingFrac, LastAppliedChargingProgress, 1e-3f))
	{
		return;
	}
	LastAppliedFullRage = FullCharges;
	LastAppliedChargingProgress = ChargingFrac;

	for (int32 i = 0; i < PipCount; ++i)
	{
		const bool bActivated = (i < FullCharges);
		const bool bCharging = (!bActivated && i == FullCharges && ChargingFrac > 1e-3f);

		UOverlay* Overlay = RageFlameOverlays.IsValidIndex(i) ? RageFlameOverlays[i].Get() : nullptr;
		UImage* GlowImg = RageGlowImages.IsValidIndex(i) ? RageGlowImages[i].Get() : nullptr;
		UImage* FlameImg = RageFlameImages.IsValidIndex(i) ? RageFlameImages[i].Get() : nullptr;
		UProgressBar* ChargeFill = RageChargingFillBars.IsValidIndex(i) ? RageChargingFillBars[i].Get() : nullptr;
		UProgressBar* MiniGauge = RageMiniGaugeBars.IsValidIndex(i) ? RageMiniGaugeBars[i].Get() : nullptr;

		if (bActivated)
		{
			// 整体激活态：金色辉光 + 放大炽亮金焰 + 满格金条 -> 一眼辨识可用点数
			if (Overlay && (!RageActivationFlashTimers.IsValidIndex(i) || RageActivationFlashTimers[i] <= 0.0f))
			{
				Overlay->SetRenderScale(FVector2D(1.08f, 1.08f));
			}
			if (GlowImg)
			{
				GlowImg->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				GlowImg->SetColorAndOpacity(FLinearColor(1.0f, 0.85f, 0.40f, 0.92f));
			}
			if (FlameImg)
			{
				if (RageOnTexture)
				{
					FlameImg->SetBrushFromTexture(RageOnTexture, false);
				}
				FlameImg->SetColorAndOpacity(FLinearColor(1.18f, 1.06f, 0.92f, 1.0f));
			}
			if (ChargeFill)
			{
				ChargeFill->SetVisibility(ESlateVisibility::Collapsed);
				ChargeFill->SetPercent(0.0f);
			}
			if (MiniGauge)
			{
				MiniGauge->SetPercent(1.0f);
				MiniGauge->SetFillColorAndOpacity(FLinearColor(1.0f, 0.86f, 0.36f, 1.0f));
			}
		}
		else if (bCharging)
		{
			// 积累过程态：无光晕 + 偏小暗壳 + 自下而上余烬液面 + 底部琥珀分段进度条 -> 明确未激活
			if (Overlay)
			{
				Overlay->SetRenderScale(FVector2D(0.88f, 0.88f));
			}
			if (GlowImg)
			{
				GlowImg->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (FlameImg)
			{
				if (RageOffTexture)
				{
					FlameImg->SetBrushFromTexture(RageOffTexture, false);
				}
				FlameImg->SetColorAndOpacity(FLinearColor(0.58f, 0.56f, 0.54f, 0.90f));
			}
			if (ChargeFill)
			{
				ChargeFill->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				ChargeFill->SetPercent(ChargingFrac);
			}
			if (MiniGauge)
			{
				MiniGauge->SetPercent(ChargingFrac);
				MiniGauge->SetFillColorAndOpacity(FLinearColor(0.82f, 0.42f, 0.14f, 0.92f));
			}
		}
		else
		{
			// 空槽态：无光晕 + 暗灰未点燃
			if (Overlay)
			{
				Overlay->SetRenderScale(FVector2D(0.86f, 0.86f));
			}
			if (GlowImg)
			{
				GlowImg->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (FlameImg)
			{
				if (RageOffTexture)
				{
					FlameImg->SetBrushFromTexture(RageOffTexture, false);
				}
				FlameImg->SetColorAndOpacity(FLinearColor(0.46f, 0.46f, 0.46f, 0.68f));
			}
			if (ChargeFill)
			{
				ChargeFill->SetVisibility(ESlateVisibility::Collapsed);
				ChargeFill->SetPercent(0.0f);
			}
			if (MiniGauge)
			{
				MiniGauge->SetPercent(0.0f);
				MiniGauge->SetFillColorAndOpacity(FLinearColor(0.25f, 0.22f, 0.20f, 0.6f));
			}
		}
	}
}

void UCombatHUDWidget::UpdateRageActivationFlash(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	constexpr float FlashDuration = 0.35f;
	for (int32 i = 0; i < RageActivationFlashTimers.Num() && i < RageFlameOverlays.Num(); ++i)
	{
		if (RageActivationFlashTimers[i] <= 0.0f)
		{
			continue;
		}
		RageActivationFlashTimers[i] = FMath::Max(0.0f, RageActivationFlashTimers[i] - DeltaTime);
		const float Alpha = RageActivationFlashTimers[i] / FlashDuration;
		const float PopScale = 1.08f + 0.24f * FMath::Sin(Alpha * PI);
		if (UOverlay* Overlay = RageFlameOverlays[i])
		{
			Overlay->SetRenderScale(FVector2D(PopScale, PopScale));
		}
		if (RageGlowImages.IsValidIndex(i) && IsValid(RageGlowImages[i]))
		{
			const float GlowBoost = 0.92f + 0.35f * Alpha;
			RageGlowImages[i]->SetColorAndOpacity(FLinearColor(1.0f, 0.90f, 0.50f, FMath::Clamp(GlowBoost, 0.0f, 1.0f)));
		}
	}
}

void UCombatHUDWidget::RefreshStaminaZoneLayout(const TArray<int32>& ZoneCapacities)
{
	EnsureHudLayoutBuilt();
	if (StaminaZoneOverlays.Num() != HeroStaminaDefaults::ZoneCount
		|| StaminaZoneSlots.Num() != HeroStaminaDefaults::ZoneCount)
	{
		return;
	}

	static const int32 Defaults[HeroStaminaDefaults::ZoneCount] = {
		HeroStaminaDefaults::DefaultExhaustedCapacity,
		HeroStaminaDefaults::DefaultFatiguedCapacity,
		HeroStaminaDefaults::DefaultNormalCapacity,
		HeroStaminaDefaults::DefaultVigorousCapacity
	};

	int32 Normalized[HeroStaminaDefaults::ZoneCount];
	bool bChanged = false;
	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		Normalized[i] = ZoneCapacities.IsValidIndex(i) ? FMath::Max(0, ZoneCapacities[i]) : Defaults[i];
		if (Normalized[i] != LastAppliedZoneCapacities[i])
		{
			bChanged = true;
		}
	}

	if (!bChanged)
	{
		return;
	}

	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount; ++i)
	{
		LastAppliedZoneCapacities[i] = Normalized[i];
		const bool bVisible = Normalized[i] > 0;
		if (UOverlay* Overlay = StaminaZoneOverlays[i])
		{
			Overlay->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (UHorizontalBoxSlot* HSlot = StaminaZoneSlots[i])
		{
			FSlateChildSize FillSize(ESlateSizeRule::Fill);
			FillSize.Value = bVisible ? static_cast<float>(Normalized[i]) : 0.0f;
			HSlot->SetSize(FillSize);
		}
	}

	// 仅在左右均存在有效可见区域时显示分割线
	for (int32 d = 0; d < StaminaZoneDividers.Num(); ++d)
	{
		bool bHasLeft = false;
		for (int32 l = 0; l <= d; ++l)
		{
			if (Normalized[l] > 0)
			{
				bHasLeft = true;
				break;
			}
		}
		bool bHasRight = false;
		for (int32 r = d + 1; r < HeroStaminaDefaults::ZoneCount; ++r)
		{
			if (Normalized[r] > 0)
			{
				bHasRight = true;
				break;
			}
		}
		if (UBorder* Div = StaminaZoneDividers[d])
		{
			Div->SetVisibility((bHasLeft && bHasRight) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
	}
}

void UCombatHUDWidget::ApplyDesignTimePreview()
{
	if (IsValid(EnemyBarContainer))
	{
		EnemyBarContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (IsValid(EnemyHealthFillBar))
	{
		EnemyHealthFillBar->SetPercent(0.65f);
	}
	if (IsValid(EnemyNameText))
	{
		EnemyNameText->SetText(FText::FromString(TEXT("骷髏戰士")));
	}
	if (IsValid(PlayerHealthFillBar))
	{
		PlayerHealthFillBar->SetPercent(0.82f);
	}
	if (IsValid(HintSkillQLabelText))
	{
		HintSkillQLabelText->SetText(FText::FromString(TEXT("盾牌冲锋")));
	}
	if (IsValid(HintSkillELabelText))
	{
		HintSkillELabelText->SetText(FText::FromString(TEXT("盾牌投掷")));
	}
	if (IsValid(HintDodgeLabelText))
	{
		HintDodgeLabelText->SetText(DodgeHintText);
	}
	if (IsValid(HintJumpLabelText))
	{
		HintJumpLabelText->SetText(JumpHintText);
	}

	// 预览：2 点完全激活 + 第 3 点积累 60%，直观看清激活与积累区别
	UpdateRageFlameDisplay(2.6f, 5);

	const TArray<int32> DefaultCaps = {
		HeroStaminaDefaults::DefaultExhaustedCapacity,
		HeroStaminaDefaults::DefaultFatiguedCapacity,
		HeroStaminaDefaults::DefaultNormalCapacity,
		HeroStaminaDefaults::DefaultVigorousCapacity
	};
	RefreshStaminaZoneLayout(DefaultCaps);
	for (int32 i = 0; i < StaminaZoneBars.Num(); ++i)
	{
		if (UProgressBar* Bar = StaminaZoneBars[i])
		{
			Bar->SetPercent(1.0f);
		}
	}
}

void UCombatHUDWidget::RefreshHUDFromHero(AHeroCharacter* InHero)
{
	EnsureHudLayoutBuilt();
	if (!IsValid(InHero))
	{
		return;
	}

	// 1. 玩家血条
	if (IsValid(PlayerHealthFillBar) && IsValid(InHero->CharacterDataComp))
	{
		const int32 MaxHP = InHero->CharacterDataComp->GetMaxHP();
		const int32 CurHP = InHero->CharacterDataComp->GetCurrentHP();
		const float HPPercent = (MaxHP > 0) ? FMath::Clamp(static_cast<float>(CurHP) / static_cast<float>(MaxHP), 0.0f, 1.0f) : 0.0f;
		PlayerHealthFillBar->SetPercent(HPPercent);
	}

	// 2. 怒气火焰（区分整体激活可用点与积累进度）
	UpdateRageFlameDisplay(InHero->GetCurrentRage(), InHero->MaxRageCharges);

	// 3. 四段动态比例体力条
	const TArray<int32> ZoneCaps = InHero->GetNormalizedStaminaZoneCapacities();
	RefreshStaminaZoneLayout(ZoneCaps);
	for (int32 i = 0; i < HeroStaminaDefaults::ZoneCount && i < StaminaZoneBars.Num(); ++i)
	{
		if (UProgressBar* Bar = StaminaZoneBars[i])
		{
			const EStaminaZone Zone = static_cast<EStaminaZone>(i);
			Bar->SetPercent(InHero->GetStaminaZoneFillPercent(Zone));
		}
	}

	// 4. 右上角动态键位提示（Q/E 从技能表读取技能名，Shift=闪避，Space=跳跃；无力区时微暗提示）
	if (IsValid(HintSkillQLabelText))
	{
		const FString QName = InHero->GetSkillQDisplayName();
		HintSkillQLabelText->SetText(FText::FromString(QName.IsEmpty() ? TEXT("技能一") : QName));
	}
	if (IsValid(HintSkillELabelText))
	{
		const FString EName = InHero->GetSkillEDisplayName();
		HintSkillELabelText->SetText(FText::FromString(EName.IsEmpty() ? TEXT("技能二") : EName));
	}
	if (IsValid(HintDodgeLabelText))
	{
		HintDodgeLabelText->SetText(DodgeHintText);
	}
	if (IsValid(HintJumpLabelText))
	{
		HintJumpLabelText->SetText(JumpHintText);
	}
	const bool bExhausted = (InHero->GetCurrentStaminaZone() == EStaminaZone::Exhausted);
	const bool bSkillQReady = !bExhausted && InHero->HasEnoughRage(InHero->GetSkillRageCostByID(InHero->SkillQ_ID));
	const bool bSkillEReady = !bExhausted && InHero->HasEnoughRage(InHero->GetSkillRageCostByID(InHero->SkillE_ID));
	const bool RowUsable[4] = { bSkillQReady, bSkillEReady, !bExhausted, !bExhausted };
	for (int32 i = 0; i < ControlHintRows.Num() && i < 4; ++i)
	{
		if (IsValid(ControlHintRows[i]))
		{
			ControlHintRows[i]->SetRenderOpacity(RowUsable[i] ? 1.0f : 0.42f);
		}
	}

	// 5. 顶部敌人血条与名称
	ACombatCharacter* Enemy = ResolveEnemyTarget(InHero);
	if (IsValid(EnemyBarContainer))
	{
		if (IsValid(Enemy) && Enemy->IsAlive() && IsValid(Enemy->CharacterDataComp))
		{
			EnemyBarContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (IsValid(EnemyHealthFillBar))
			{
				const int32 MaxHP = Enemy->CharacterDataComp->GetMaxHP();
				const int32 CurHP = Enemy->CharacterDataComp->GetCurrentHP();
				const float EnemyPct = (MaxHP > 0) ? FMath::Clamp(static_cast<float>(CurHP) / static_cast<float>(MaxHP), 0.0f, 1.0f) : 0.0f;
				EnemyHealthFillBar->SetPercent(EnemyPct);
			}
			if (IsValid(EnemyNameText))
			{
				EnemyNameText->SetText(FText::FromString(Enemy->CharacterDataComp->displayName));
			}
		}
		else
		{
			EnemyBarContainer->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

float UCombatHUDWidget::GetAppliedStaminaZoneSlotWeight(EStaminaZone Zone) const
{
	const int32 Index = static_cast<int32>(Zone);
	if (StaminaZoneSlots.IsValidIndex(Index) && IsValid(StaminaZoneSlots[Index]))
	{
		return StaminaZoneSlots[Index]->GetSize().Value;
	}
	return 0.0f;
}

float UCombatHUDWidget::GetAppliedStaminaZonePercent(EStaminaZone Zone) const
{
	const int32 Index = static_cast<int32>(Zone);
	if (StaminaZoneBars.IsValidIndex(Index) && IsValid(StaminaZoneBars[Index]))
	{
		return StaminaZoneBars[Index]->GetPercent();
	}
	return 0.0f;
}

FString UCombatHUDWidget::GetHintSkillQText() const
{
	return IsValid(HintSkillQLabelText) ? HintSkillQLabelText->GetText().ToString() : FString();
}

FString UCombatHUDWidget::GetHintSkillEText() const
{
	return IsValid(HintSkillELabelText) ? HintSkillELabelText->GetText().ToString() : FString();
}

FString UCombatHUDWidget::GetHintDodgeText() const
{
	return IsValid(HintDodgeLabelText) ? HintDodgeLabelText->GetText().ToString() : FString();
}

FString UCombatHUDWidget::GetHintJumpText() const
{
	return IsValid(HintJumpLabelText) ? HintJumpLabelText->GetText().ToString() : FString();
}

bool UCombatHUDWidget::IsRagePipActivated(int32 Index) const
{
	return Index >= 0 && Index < LastAppliedFullRage;
}

bool UCombatHUDWidget::IsRagePipCharging(int32 Index) const
{
	return Index >= 0 && Index == LastAppliedFullRage && LastAppliedChargingProgress > 1e-3f;
}

AHeroCharacter* UCombatHUDWidget::ResolveHeroCharacter() const
{
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		if (AHeroCharacter* Hero = Cast<AHeroCharacter>(OwningPawn))
		{
			return Hero;
		}
	}
	if (APlayerController* PC = GetOwningPlayer())
	{
		return Cast<AHeroCharacter>(PC->GetPawn());
	}
	return nullptr;
}

ACombatCharacter* UCombatHUDWidget::ResolveEnemyTarget(AHeroCharacter* Hero) const
{
	// 1. 优先读取锁定/选中的战斗目标
	if (IsValid(Hero) && IsValid(Hero->TargetingComp))
	{
		if (ACombatCharacter* Targeted = Cast<ACombatCharacter>(Hero->TargetingComp->GetSelectedTarget()))
		{
			if (Targeted != Hero && Targeted->IsAlive())
			{
				return Targeted;
			}
		}
	}

	// 2. 兼容现有怪物蓝图（BP_Wolf / BP_SkeletonLight）直接写入 WBP_CombatCharater.character 变量
	if (ACombatCharacter* Fallback = GetBlueprintFallbackEnemyCharacter())
	{
		if (Fallback != Hero && Fallback->IsAlive())
		{
			return Fallback;
		}
	}

	return nullptr;
}

ACombatCharacter* UCombatHUDWidget::GetBlueprintFallbackEnemyCharacter() const
{
	if (FObjectPropertyBase* Prop = FindFProperty<FObjectPropertyBase>(GetClass(), TEXT("character")))
	{
		return Cast<ACombatCharacter>(Prop->GetObjectPropertyValue_InContainer(this));
	}
	return nullptr;
}
