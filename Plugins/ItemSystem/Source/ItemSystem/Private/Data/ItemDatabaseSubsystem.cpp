#include "Data/ItemDatabaseSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "ImageUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogItemDatabase, Log, All);

void UItemDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ItemSystem")))
	{
		ContentDir = Plugin->GetContentDir();
		LoadCsv(FPaths::Combine(ContentDir, TEXT("Data/ItemBase.csv")));
	}
	else
	{
		UE_LOG(LogItemDatabase, Error, TEXT("找不到 ItemSystem 插件目录"));
	}
}

UItemDatabaseSubsystem* UItemDatabaseSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;
}

void UItemDatabaseSubsystem::LoadCsv(const FString& FilePath)
{
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath) || Lines.Num() < 2)
	{
		UE_LOG(LogItemDatabase, Error, TEXT("读取道具表失败: %s"), *FilePath);
		return;
	}

	TArray<FString> Header;
	Lines[0].ParseIntoArray(Header, TEXT(","), false);
	auto Col = [&Header](const TCHAR* Name) { return Header.IndexOfByKey(FString(Name)); };
	const int32 cID = Col(TEXT("ID")), cName = Col(TEXT("Name")), cEquip = Col(TEXT("EquipType")), cLevel = Col(TEXT("Level"));
	const int32 cAc = Col(TEXT("Ac")), cDc = Col(TEXT("Dc")), cDc2 = Col(TEXT("Dc2")), cBlock = Col(TEXT("Block"));
	const int32 cW = Col(TEXT("CellSizeX")), cH = Col(TEXT("CellSizeY")), cIcon = Col(TEXT("Icon")), cDesc = Col(TEXT("Desc"));

	for (int32 i = 1; i < Lines.Num(); ++i)
	{
		const FString Line = Lines[i].TrimStartAndEnd();
		if (Line.IsEmpty()) continue;

		TArray<FString> F;
		Line.ParseIntoArray(F, TEXT(","), false);
		auto Str = [&F](int32 Idx) { return F.IsValidIndex(Idx) ? F[Idx].TrimStartAndEnd() : FString(); };
		auto Int = [&Str](int32 Idx) { return FCString::Atoi(*Str(Idx)); };

		FItemBaseRow Row;
		Row.ID = Int(cID);
		if (Row.ID <= 0) continue;
		Row.Name = Str(cName);
		Row.EquipType = Int(cEquip);
		Row.Level = Int(cLevel);
		Row.Ac = Int(cAc);
		Row.Dc = Int(cDc);
		Row.Dc2 = Int(cDc2);
		Row.Block = Int(cBlock);
		Row.CellSizeX = Int(cW);
		Row.CellSizeY = Int(cH);
		Row.Icon = Str(cIcon);
		Row.Desc = Str(cDesc);

		Rows.Add(Row.ID, Row);
		OrderedIDs.Add(Row.ID);
	}
	UE_LOG(LogItemDatabase, Log, TEXT("道具表已加载 %d 条"), Rows.Num());
}

FIntPoint UItemDatabaseSubsystem::GetCellSize(int32 BaseID) const
{
	const FItemBaseRow* Row = Find(BaseID);
	return Row ? Row->GetCellSize() : FIntPoint(1, 1);
}

UTexture2D* UItemDatabaseSubsystem::GetIcon(int32 BaseID)
{
	if (TObjectPtr<UTexture2D>* Cached = IconCache.Find(BaseID))
	{
		return *Cached;
	}
	UTexture2D* Tex = nullptr;
	if (const FItemBaseRow* Row = Find(BaseID))
	{
		if (!Row->Icon.IsEmpty())
		{
			// 临时方案：运行时直接读 PNG。以后导入成 UTexture2D 资源后改为软引用加载。
			Tex = FImageUtils::ImportFileAsTexture2D(FPaths::Combine(ContentDir, Row->Icon));
			if (!Tex)
			{
				UE_LOG(LogItemDatabase, Warning, TEXT("图标加载失败: %d %s"), BaseID, *Row->Icon);
			}
		}
	}
	IconCache.Add(BaseID, Tex);
	return Tex;
}

FItemInstance UItemDatabaseSubsystem::CreateInstance(int32 BaseID) const
{
	FItemInstance Item;
	if (Find(BaseID))
	{
		Item.InstanceId = FGuid::NewGuid();
		Item.BaseID = BaseID;
	}
	return Item;
}
