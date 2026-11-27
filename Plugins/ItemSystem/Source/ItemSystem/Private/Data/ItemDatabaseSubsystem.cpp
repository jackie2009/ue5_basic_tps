#include "Data/ItemDatabaseSubsystem.h"

#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "ImageUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogItemDatabase, Log, All);

namespace ItemDatabaseInternal
{
	static TArray<FIntPoint> ParseExtraAttributes(const FString& Raw)
	{
		TArray<FIntPoint> Out;
		if (Raw.IsEmpty()) return Out;

		TArray<FString> Pairs;
		Raw.ParseIntoArray(Pairs, TEXT(";"), true);
		for (const FString& Pair : Pairs)
		{
			FString Left, Right;
			if (Pair.Split(TEXT(":"), &Left, &Right))
			{
				const int32 AttrID = FCString::Atoi(*Left.TrimStartAndEnd());
				const int32 Value = FCString::Atoi(*Right.TrimStartAndEnd());
				if (AttrID > 0 && Value != 0)
				{
					Out.Add(FIntPoint(AttrID, Value));
				}
			}
		}
		return Out;
	}
}

void UItemDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ItemSystem")))
	{
		ContentDir = Plugin->GetContentDir();
		ReloadAll();
	}
	else
	{
		UE_LOG(LogItemDatabase, Error, TEXT("找不到 ItemSystem 插件目录"));
	}
}

void UItemDatabaseSubsystem::ReloadAll()
{
	if (ContentDir.IsEmpty())
	{
		if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ItemSystem")))
		{
			ContentDir = Plugin->GetContentDir();
		}
	}
	if (ContentDir.IsEmpty()) return;

	Rows.Empty();
	OrderedIDs.Empty();
	AffixRows.Empty();
	EnabledAffixesByEquipType.Empty();

	LoadCsv(FPaths::Combine(ContentDir, TEXT("Data/ItemBase.csv")));
	LoadAffixesJson(FPaths::Combine(ContentDir, TEXT("Data/RandomAffixes.json")));
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
	for (FString& H : Header)
	{
		H = H.TrimStartAndEnd();
	}

	auto Col = [&Header](const TCHAR* Name) { return Header.IndexOfByKey(FString(Name)); };
	const int32 cID = Col(TEXT("ID"));
	const int32 cName = Col(TEXT("Name"));
	const int32 cEquip = Col(TEXT("EquipType"));
	const int32 cLevel = Col(TEXT("Level"));
	const int32 cAc = Col(TEXT("Ac"));
	const int32 cDc = Col(TEXT("Dc"));
	const int32 cDc2 = Col(TEXT("Dc2"));
	const int32 cBlock = Col(TEXT("Block"));
	const int32 cW = Col(TEXT("CellSizeX"));
	const int32 cH = Col(TEXT("CellSizeY"));
	const int32 cIcon = Col(TEXT("Icon"));
	const int32 cDesc = Col(TEXT("Desc"));
	const int32 cQualityMin = Col(TEXT("QualityMin"));
	const int32 cHiScale = Col(TEXT("HiQualityScale"));
	const int32 cExtraAttrs = Col(TEXT("ExtraAttributes"));
	const int32 cDropWeight = Col(TEXT("DropWeight"));
	const int32 cCanWorldDrop = Col(TEXT("CanWorldDrop"));

	for (int32 i = 1; i < Lines.Num(); ++i)
	{
		const FString Line = Lines[i].TrimStartAndEnd();
		if (Line.IsEmpty()) continue;

		TArray<FString> F;
		Line.ParseIntoArray(F, TEXT(","), false);
		auto Str = [&F](int32 Idx) { return F.IsValidIndex(Idx) ? F[Idx].TrimStartAndEnd() : FString(); };
		auto Int = [&Str](int32 Idx, int32 DefaultVal = 0)
		{
			const FString S = Str(Idx);
			return S.IsEmpty() ? DefaultVal : FCString::Atoi(*S);
		};

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
		Row.CellSizeX = Int(cW, 1);
		Row.CellSizeY = Int(cH, 1);
		Row.Icon = Str(cIcon);
		Row.Desc = Str(cDesc);
		Row.QualityMin = cQualityMin >= 0 ? Int(cQualityMin, 0) : 0;
		Row.HiQualityScale = cHiScale >= 0 ? Int(cHiScale, 100) : 100;
		Row.ExtraAttributes = cExtraAttrs >= 0 ? ItemDatabaseInternal::ParseExtraAttributes(Str(cExtraAttrs)) : TArray<FIntPoint>();
		Row.DropWeight = cDropWeight >= 0 ? FMath::Max(0, Int(cDropWeight, 100)) : 100;
		Row.bCanWorldDrop = cCanWorldDrop >= 0 ? (Int(cCanWorldDrop, 1) != 0) : true;

		Rows.Add(Row.ID, Row);
		OrderedIDs.Add(Row.ID);
	}
	UE_LOG(LogItemDatabase, Log, TEXT("道具表已加载 %d 条"), Rows.Num());
}

void UItemDatabaseSubsystem::LoadAffixesJson(const FString& FilePath)
{
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		UE_LOG(LogItemDatabase, Warning, TEXT("未找到随机词缀表: %s"), *FilePath);
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogItemDatabase, Error, TEXT("解析随机词缀 JSON 失败: %s"), *FilePath);
		return;
	}

	int32 CapLevel = 42;
	if (Root->TryGetNumberField(TEXT("AttackPercentAffixMaxLevel"), CapLevel))
	{
		AttackPercentAffixMaxLevel = CapLevel;
	}

	const TArray<TSharedPtr<FJsonValue>>* AffixesArray = nullptr;
	if (!Root->TryGetArrayField(TEXT("Affixes"), AffixesArray) || !AffixesArray)
	{
		UE_LOG(LogItemDatabase, Error, TEXT("随机词缀 JSON 缺少 Affixes 数组: %s"), *FilePath);
		return;
	}

	for (const TSharedPtr<FJsonValue>& Val : *AffixesArray)
	{
		const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
		if (!Val.IsValid() || !Val->TryGetObject(ObjPtr) || !ObjPtr || !ObjPtr->IsValid()) continue;
		const TSharedPtr<FJsonObject>& Obj = *ObjPtr;

		FRandomAffixRow Row;
		Row.ID = static_cast<int32>(Obj->GetNumberField(TEXT("ID")));
		if (Row.ID <= 0) continue;

		Obj->TryGetStringField(TEXT("Name"), Row.Name);
		Obj->TryGetBoolField(TEXT("bEnable"), Row.bEnable);
		Obj->TryGetBoolField(TEXT("bPercentData"), Row.bPercentData);
		Obj->TryGetStringField(TEXT("PreName"), Row.PreName);
		Obj->TryGetStringField(TEXT("LastName"), Row.LastName);

		auto ReadIntArray = [&](const TCHAR* FieldName, TArray<int32>& OutArr)
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
			if (Obj->TryGetArrayField(FieldName, Arr) && Arr)
			{
				OutArr.Reserve(Arr->Num());
				for (const TSharedPtr<FJsonValue>& V : *Arr)
				{
					OutArr.Add(V.IsValid() ? static_cast<int32>(V->AsNumber()) : 0);
				}
			}
		};
		ReadIntArray(TEXT("MinValues"), Row.MinValues);
		ReadIntArray(TEXT("MaxValues"), Row.MaxValues);

		AffixRows.Add(Row.ID, Row);
		if (Row.bEnable && Row.MinValues.Num() >= 2 && Row.MaxValues.Num() >= 2)
		{
			EnabledAffixesByEquipType.FindOrAdd(Row.GetEquipType()).Add(Row.ID);
		}
	}

	UE_LOG(LogItemDatabase, Log, TEXT("随机词缀表已加载 %d 条（覆盖 %d 个部位）"), AffixRows.Num(), EnabledAffixesByEquipType.Num());
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
	const FItemBaseRow* Row = Find(BaseID);
	if (!Row) return FItemInstance();
	FRandomStream Rng(FMath::Rand());
	return CreateRolledInstance(BaseID, Row->QualityMin, Rng);
}

FItemInstance UItemDatabaseSubsystem::CreateInstanceWithQuality(int32 BaseID, int32 TargetQuality) const
{
	FRandomStream Rng(FMath::Rand());
	return CreateRolledInstance(BaseID, TargetQuality, Rng);
}

FItemInstance UItemDatabaseSubsystem::CreateRolledInstance(int32 BaseID, int32 TargetQuality, FRandomStream& Rng) const
{
	FItemInstance Item;
	const FItemBaseRow* Row = Find(BaseID);
	if (!Row) return Item;

	Item.InstanceId = FGuid::NewGuid();
	Item.BaseID = BaseID;
	Item.Count = 1;
	Item.Quality = Row->IsEquipment() ? FMath::Clamp(FMath::Max(TargetQuality, Row->QualityMin), 0, 4) : FMath::Max(0, Row->QualityMin);

	if (Row->IsEquipment())
	{
		RollAffixesForInstance(*Row, Item, Rng);
	}

	// 合并固定额外属性（暗金/特殊装备自带）
	for (const FIntPoint& Extra : Row->ExtraAttributes)
	{
		const int32 Key = Row->EquipType * 1000 + Extra.X;
		Item.DynamicAttributes.FindOrAdd(Key) += Extra.Y;
	}

	return Item;
}

int32 UItemDatabaseSubsystem::PickRandomEnabledAffixID(int32 EquipType, const TMap<int32, int32>& Existing, FRandomStream& Rng) const
{
	const TArray<int32>* Pool = EnabledAffixesByEquipType.Find(EquipType);
	if (!Pool || Pool->Num() == 0) return 0;

	TArray<int32> Candidates;
	Candidates.Reserve(Pool->Num());
	for (int32 AffixID : *Pool)
	{
		if (!Existing.Contains(AffixID))
		{
			Candidates.Add(AffixID);
		}
	}
	if (Candidates.Num() == 0) return 0;

	const int32 PickIdx = Rng.RandRange(0, Candidates.Num() - 1);
	return Candidates[PickIdx];
}

int32 UItemDatabaseSubsystem::RollSingleAffixValue(
	const FRandomAffixRow& Affix, int32 ItemLevel, int32 Quality, int32 ScalePercent, FRandomStream& Rng) const
{
	if (Affix.MinValues.Num() < 2 || Affix.MaxValues.Num() < 2) return 0;

	int32 EffectiveLevel = FMath::Max(0, ItemLevel);
	if (Affix.GetAttrID() == static_cast<int32>(EItemAffixAttr::AttackPercent) && AttackPercentAffixMaxLevel > 0)
	{
		EffectiveLevel = FMath::Min(EffectiveLevel, AttackPercentAffixMaxLevel);
	}

	const int32 MaxTierIndex = FMath::Min(Affix.MinValues.Num(), Affix.MaxValues.Num()) - 1;
	const float StepLevel = FMath::Clamp(EffectiveLevel / 10.0f, 0.0f, static_cast<float>(MaxTierIndex));
	const int32 StartIdx = FMath::Clamp(FMath::FloorToInt(StepLevel), 0, MaxTierIndex - 1);
	const float Alpha = FMath::Clamp(StepLevel - static_cast<float>(StartIdx), 0.0f, 1.0f);

	int32 MinVal = FMath::RoundToInt(FMath::Lerp(static_cast<float>(Affix.MinValues[StartIdx]), static_cast<float>(Affix.MinValues[StartIdx + 1]), Alpha));
	int32 MaxVal = FMath::RoundToInt(FMath::Lerp(static_cast<float>(Affix.MaxValues[StartIdx]), static_cast<float>(Affix.MaxValues[StartIdx + 1]), Alpha));

	if (Quality >= static_cast<int32>(EItemQuality::Rare))
	{
		const int32 Span = FMath::Max(0, MaxVal - MinVal);
		const int32 Bump = FMath::RoundToInt(Span * 0.2f);
		MinVal += Bump;
		MaxVal += Bump;

		const int32 SafeScale = ScalePercent > 0 ? ScalePercent : 100;
		MinVal = MinVal * SafeScale / 100;
		MaxVal = MaxVal * SafeScale / 100;

		// 稀有下限保底 80%，暗金下限保底 85%（与旧项目 PropInstVo 一致）
		const int32 FloorPct = (Quality >= static_cast<int32>(EItemQuality::Unique)) ? 85 : 80;
		MinVal = FMath::Max(MinVal, (MaxVal * FloorPct + 99) / 100);
	}

	if (MaxVal < MinVal)
	{
		Swap(MinVal, MaxVal);
	}
	return Rng.RandRange(MinVal, MaxVal);
}

void UItemDatabaseSubsystem::RollAffixesForInstance(const FItemBaseRow& Row, FItemInstance& Item, FRandomStream& Rng) const
{
	if (Item.Quality <= static_cast<int32>(EItemQuality::Normal)) return;
	if (!EnabledAffixesByEquipType.Contains(Row.EquipType)) return;

	if (Item.Quality == static_cast<int32>(EItemQuality::Magic))
	{
		// 25% 单前缀，50% 单后缀，25% 双词缀
		const int32 Roll = Rng.RandRange(0, 99);
		const int32 PropCount = (Roll >= 75) ? 2 : 1;
		for (int32 i = 0; i < PropCount; ++i)
		{
			const int32 Key = PickRandomEnabledAffixID(Row.EquipType, Item.DynamicAttributes, Rng);
			const FRandomAffixRow* Affix = FindAffix(Key);
			if (!Affix) break;
			const int32 Value = RollSingleAffixValue(*Affix, Row.Level, Item.Quality, 100, Rng);
			Item.DynamicAttributes.Add(Key, Value);
		}
		return;
	}

	// Rare(2) = 3 条，Unique(3) = 4 条
	const bool bIsUnique = (Item.Quality >= static_cast<int32>(EItemQuality::Unique));
	const int32 PropCount = bIsUnique ? 4 : 3;
	const int32 Scale = bIsUnique ? (Row.HiQualityScale > 100 ? Row.HiQualityScale : 125) : 100;

	const int32 GuaranteedWeaponKey = Row.EquipType * 1000 + static_cast<int32>(EItemAffixAttr::AttackPercent);
	const FRandomAffixRow* GuaranteedWeaponAffix = Row.IsWeapon() ? FindAffix(GuaranteedWeaponKey) : nullptr;
	const bool bGuaranteeWeaponAtk = GuaranteedWeaponAffix && GuaranteedWeaponAffix->bEnable;

	for (int32 i = 0; i < PropCount; ++i)
	{
		const int32 Key = (bGuaranteeWeaponAtk && i == 0)
			? GuaranteedWeaponKey
			: PickRandomEnabledAffixID(Row.EquipType, Item.DynamicAttributes, Rng);
		const FRandomAffixRow* Affix = FindAffix(Key);
		if (!Affix) break;
		const int32 Value = RollSingleAffixValue(*Affix, Row.Level, Item.Quality, Scale, Rng);
		Item.DynamicAttributes.Add(Key, Value);
	}
}

FString UItemDatabaseSubsystem::GetQualityHexColor(int32 Quality)
{
	switch (Quality)
	{
	case 1: return TEXT("4497B8"); // 魔法蓝
	case 2: return TEXT("DFB43D"); // 稀有金/黄
	case 3: return TEXT("EE7B47"); // 暗金橙
	case 4: return TEXT("76BF89"); // 套装绿
	default: return TEXT("FFFFFF"); // 普通白
	}
}

FLinearColor UItemDatabaseSubsystem::GetQualityColor(int32 Quality)
{
	return FLinearColor::FromSRGBColor(FColor::FromHex(GetQualityHexColor(Quality)));
}

FString UItemDatabaseSubsystem::GetQualityDisplayName(int32 Quality)
{
	switch (Quality)
	{
	case 1: return TEXT("魔法");
	case 2: return TEXT("稀有");
	case 3: return TEXT("暗金");
	case 4: return TEXT("套装");
	default: return TEXT("普通");
	}
}

FString UItemDatabaseSubsystem::GetItemDisplayName(const FItemInstance& Item) const
{
	const FItemBaseRow* Row = Find(Item.BaseID);
	if (!Row) return TEXT("未知物品");

	if (Item.Quality == static_cast<int32>(EItemQuality::Magic) && Item.DynamicAttributes.Num() > 0)
	{
		TArray<int32> Keys;
		Item.DynamicAttributes.GetKeys(Keys);
		Keys.Sort();

		FString Prefix;
		FString Suffix;
		if (Keys.Num() >= 1)
		{
			if (const FRandomAffixRow* First = FindAffix(Keys[0]))
			{
				Prefix = First->PreName;
			}
		}
		if (Keys.Num() >= 2)
		{
			if (const FRandomAffixRow* Second = FindAffix(Keys[1]))
			{
				Suffix = Second->LastName;
			}
		}
		return Prefix + Suffix + Row->Name;
	}

	return Row->Name;
}

FString UItemDatabaseSubsystem::FormatAffixText(int32 AffixID, int32 Value) const
{
	if (const FRandomAffixRow* Affix = FindAffix(AffixID))
	{
		if (Affix->bPercentData)
		{
			return FString::Printf(TEXT("+%d%% %s"), Value, *Affix->Name);
		}
		return FString::Printf(TEXT("+%d %s"), Value, *Affix->Name);
	}
	return FString::Printf(TEXT("+%d 属性(%d)"), Value, AffixID % 1000);
}

FString UItemDatabaseSubsystem::FormatItemTooltip(const FItemInstance& Item) const
{
	const FItemBaseRow* Row = Find(Item.BaseID);
	if (!Row) return FString();

	const FText SlotName = Row->EquipType == static_cast<int32>(EEquipSlot::Finger1)
		? FText::FromString(TEXT("戒指"))
		: StaticEnum<EEquipSlot>()->GetDisplayNameTextByValue(Row->EquipType);

	FString Info = FString::Printf(TEXT("[%s] %s    Lv.%d    %s"),
		*GetQualityDisplayName(Item.Quality), *GetItemDisplayName(Item), Row->Level, *SlotName.ToString());

	if (Row->Ac > 0) Info += FString::Printf(TEXT("    防御 %d"), Row->Ac);
	if (Row->Dc2 > 0) Info += FString::Printf(TEXT("    攻击 %d-%d"), Row->Dc, Row->Dc2);
	if (Row->Block > 0) Info += FString::Printf(TEXT("    格挡 %d"), Row->Block);
	Info += FString::Printf(TEXT("    占格 %dx%d"), Row->GetCellSize().X, Row->GetCellSize().Y);

	if (Item.DynamicAttributes.Num() > 0)
	{
		TArray<int32> Keys;
		Item.DynamicAttributes.GetKeys(Keys);
		Keys.Sort();

		TArray<FString> AffixParts;
		AffixParts.Reserve(Keys.Num());
		for (int32 Key : Keys)
		{
			AffixParts.Add(FormatAffixText(Key, Item.DynamicAttributes[Key]));
		}
		Info += TEXT("\n词缀: ") + FString::Join(AffixParts, TEXT("  |  "));
	}

	if (!Row->Desc.IsEmpty())
	{
		Info += TEXT("\n") + Row->Desc;
	}

	return Info;
}
