#include "Widgets/ItemIconWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Data/ItemDatabaseSubsystem.h"

void UItemIconWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Apply();
}

void UItemIconWidget::SetItem(const FItemInstance& InItem)
{
	Item = InItem;
	Apply();
}

void UItemIconWidget::Apply()
{
	if (!Icon) return;
	UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(this);
	UTexture2D* Tex = (DB && Item.IsValid()) ? DB->GetIcon(Item.BaseID) : nullptr;
	Icon->SetBrushFromTexture(Tex, false);
	Icon->SetRenderOpacity(Tex ? 1.f : 0.f);

	if (Frame)
	{
		if (Item.IsValid() && Item.Quality > 0)
		{
			FLinearColor Tint = UItemDatabaseSubsystem::GetQualityColor(Item.Quality);
			Tint.A = 0.35f;
			Frame->SetBrushColor(Tint);
		}
		else
		{
			Frame->SetBrushColor(FLinearColor(0.18f, 0.16f, 0.14f, 0.55f));
		}
	}
}
