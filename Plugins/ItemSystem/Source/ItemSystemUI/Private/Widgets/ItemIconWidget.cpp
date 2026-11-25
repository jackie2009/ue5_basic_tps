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
}
