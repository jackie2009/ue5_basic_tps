#include "Loot/LootGeneratorSubsystem.h"

#include "Data/ItemDatabaseSubsystem.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogLootGenerator, Log, All);

namespace LootGeneratorInternal
{
	static constexpr int32 MaxSubPoolDepth = 4;

	static ELootPoolEntryType ParseEntryType(const FString& Str)
	{
		if (Str.Equals(TEXT("DynamicEquip"), ESearchCase::IgnoreCase)) return ELootPoolEntryType::DynamicEquip;
		if (Str.Equals(TEXT("Gold"), ESearchCase::IgnoreCase)) return ELootPoolEntryType::Gold;
		if (Str.Equals(TEXT("SubPool"), ESearchCase::IgnoreCase)) return ELootPoolEntryType::SubPool;
		return ELootPoolEntryType::FixedItem;
	}

	static FString FindPluginContentDir()
	{
		if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ItemSystem")))
		{
			return Plugin->GetContentDir();
		}
		return FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("ItemSystem/Content"));
	}
}

void ULootGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UItemDatabaseSubsystem>();

	ContentDir = LootGeneratorInternal::FindPluginContentDir();
	LoadLootRulesJson(FPaths::Combine(ContentDir, TEXT("Data/LootRules.json")));
	LoadMonsterProfilesJson(FPaths::Combine(ContentDir, TEXT("Data/MonsterLootProfiles.json")));

	const TArray<FString> Errors = ValidateConfigs();
	for (const FString& Err : Errors)
	{
		UE_LOG(LogLootGenerator, Error, TEXT("[LootConfig校验] %s"), *Err);
	}

	RegisterConsoleCommands();
}

void ULootGeneratorSubsystem::Deinitialize()
{
	UnregisterConsoleCommands();
	Super::Deinitialize();
}

ULootGeneratorSubsystem* ULootGeneratorSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<ULootGeneratorSubsystem>() : nullptr;
}

TArray<FString> ULootGeneratorSubsystem::GetAvailableProfileIdsFromDisk()
{
	TArray<FString> Result;
	Result.Add(TEXT("")); // 空表示按 DropID 或默认阶级兜底

	const FString FilePath = FPaths::Combine(LootGeneratorInternal::FindPluginContentDir(), TEXT("Data/MonsterLootProfiles.json"));
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		return Result;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return Result;
	}

	const TSharedPtr<FJsonObject>* ProfilesObj = nullptr;
	if (Root->TryGetObjectField(TEXT("Profiles"), ProfilesObj) && ProfilesObj && ProfilesObj->IsValid())
	{
		TArray<FString> Keys;
		Keys.Reserve((*ProfilesObj)->Values.Num());
		for (const auto& Pair : (*ProfilesObj)->Values)
		{
			Keys.Add(FString(*Pair.Key));
		}
		Keys.Sort();
		Result.Append(Keys);
	}
	return Result;
}

bool ULootGeneratorSubsystem::ReloadAllLootConfigs()
{
	if (UItemDatabaseSubsystem* DB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemDatabaseSubsystem>() : nullptr)
	{
		DB->ReloadAll();
	}

	ContentDir = LootGeneratorInternal::FindPluginContentDir();
	const bool bRulesOk = LoadLootRulesJson(FPaths::Combine(ContentDir, TEXT("Data/LootRules.json")));
	const bool bProfilesOk = LoadMonsterProfilesJson(FPaths::Combine(ContentDir, TEXT("Data/MonsterLootProfiles.json")));

	const TArray<FString> Errors = ValidateConfigs();
	for (const FString& Err : Errors)
	{
		UE_LOG(LogLootGenerator, Error, TEXT("[LootConfig校验] %s"), *Err);
	}

	UE_LOG(LogLootGenerator, Log, TEXT("掉落配置热重载完成：Pools=%d, Profiles=%d, DropIdMap=%d, Errors=%d"),
		Pools.Num(), Profiles.Num(), DropIdMap.Num(), Errors.Num());
	return bRulesOk && bProfilesOk && Errors.Num() == 0;
}

EMonsterLootTier ULootGeneratorSubsystem::ParseTierString(const FString& Str)
{
	if (Str.Equals(TEXT("Elite"), ESearchCase::IgnoreCase)) return EMonsterLootTier::Elite;
	if (Str.Equals(TEXT("Champion"), ESearchCase::IgnoreCase)) return EMonsterLootTier::Champion;
	if (Str.Equals(TEXT("Boss"), ESearchCase::IgnoreCase)) return EMonsterLootTier::Boss;
	return EMonsterLootTier::Normal;
}

FName ULootGeneratorSubsystem::GetDefaultProfileIdForTier(EMonsterLootTier Tier)
{
	switch (Tier)
	{
	case EMonsterLootTier::Elite: return FName(TEXT("Default_Elite"));
	case EMonsterLootTier::Champion: return FName(TEXT("Default_Champion"));
	case EMonsterLootTier::Boss: return FName(TEXT("Default_Boss"));
	default: return FName(TEXT("Default_Normal"));
	}
}

bool ULootGeneratorSubsystem::LoadLootRulesJson(const FString& FilePath)
{
	QualityPresets.Empty();
	Pools.Empty();
	MFConfig = FLootMFConfig();

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		UE_LOG(LogLootGenerator, Error, TEXT("读取 LootRules.json 失败: %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogLootGenerator, Error, TEXT("解析 LootRules.json 失败: %s"), *FilePath);
		return false;
	}

	const TSharedPtr<FJsonObject>* MFObj = nullptr;
	if (Root->TryGetObjectField(TEXT("MF"), MFObj) && MFObj && MFObj->IsValid())
	{
		double Val = 0.0;
		if ((*MFObj)->TryGetNumberField(TEXT("MagicK"), Val)) MFConfig.MagicK = static_cast<float>(Val);
		if ((*MFObj)->TryGetNumberField(TEXT("RareK"), Val)) MFConfig.RareK = static_cast<float>(Val);
		if ((*MFObj)->TryGetNumberField(TEXT("UniqueK"), Val)) MFConfig.UniqueK = static_cast<float>(Val);
	}

	const TSharedPtr<FJsonObject>* PresetsObj = nullptr;
	if (Root->TryGetObjectField(TEXT("QualityPresets"), PresetsObj) && PresetsObj && PresetsObj->IsValid())
	{
		for (const auto& Pair : (*PresetsObj)->Values)
		{
			const TSharedPtr<FJsonObject>* Obj = nullptr;
			if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(Obj) || !Obj || !Obj->IsValid()) continue;

			FLootQualityPreset Preset;
			double Val = 0.0;
			if ((*Obj)->TryGetNumberField(TEXT("NormalWeight"), Val)) Preset.NormalWeight = static_cast<float>(Val);
			if ((*Obj)->TryGetNumberField(TEXT("MagicWeight"), Val)) Preset.MagicWeight = static_cast<float>(Val);
			if ((*Obj)->TryGetNumberField(TEXT("RareWeight"), Val)) Preset.RareWeight = static_cast<float>(Val);
			if ((*Obj)->TryGetNumberField(TEXT("UniqueWeight"), Val)) Preset.UniqueWeight = static_cast<float>(Val);
			QualityPresets.Add(ParseTierString(FString(*Pair.Key)), Preset);
		}
	}

	const TSharedPtr<FJsonObject>* PoolsObj = nullptr;
	if (Root->TryGetObjectField(TEXT("Pools"), PoolsObj) && PoolsObj && PoolsObj->IsValid())
	{
		for (const auto& Pair : (*PoolsObj)->Values)
		{
			const TSharedPtr<FJsonObject>* Obj = nullptr;
			if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(Obj) || !Obj || !Obj->IsValid()) continue;

			FLootPoolConfig Pool;
			double NoDrop = 0.0;
			if ((*Obj)->TryGetNumberField(TEXT("NoDropWeight"), NoDrop))
			{
				Pool.NoDropWeight = FMath::Max(0.0f, static_cast<float>(NoDrop));
			}

			const TArray<TSharedPtr<FJsonValue>>* EntriesArr = nullptr;
			if ((*Obj)->TryGetArrayField(TEXT("Entries"), EntriesArr) && EntriesArr)
			{
				for (const TSharedPtr<FJsonValue>& EntryVal : *EntriesArr)
				{
					const TSharedPtr<FJsonObject>* EObj = nullptr;
					if (!EntryVal.IsValid() || !EntryVal->TryGetObject(EObj) || !EObj || !EObj->IsValid()) continue;

					FLootPoolEntry Entry;
					FString TypeStr;
					(*EObj)->TryGetStringField(TEXT("Type"), TypeStr);
					Entry.Type = LootGeneratorInternal::ParseEntryType(TypeStr);

					double Num = 0.0;
					if ((*EObj)->TryGetNumberField(TEXT("Weight"), Num)) Entry.Weight = FMath::Max(0.0f, static_cast<float>(Num));
					if ((*EObj)->TryGetNumberField(TEXT("ItemID"), Num)) Entry.ItemID = static_cast<int32>(Num);
					if ((*EObj)->TryGetNumberField(TEXT("ForcedQuality"), Num)) Entry.ForcedQuality = static_cast<int32>(Num);
					if ((*EObj)->TryGetNumberField(TEXT("LevelMinOffset"), Num)) Entry.LevelMinOffset = static_cast<int32>(Num);
					if ((*EObj)->TryGetNumberField(TEXT("LevelMaxOffset"), Num)) Entry.LevelMaxOffset = static_cast<int32>(Num);
					if ((*EObj)->TryGetNumberField(TEXT("GoldScaleMin"), Num)) Entry.GoldScaleMin = static_cast<float>(Num);
					if ((*EObj)->TryGetNumberField(TEXT("GoldScaleMax"), Num)) Entry.GoldScaleMax = static_cast<float>(Num);

					FString SubPoolStr;
					if ((*EObj)->TryGetStringField(TEXT("SubPoolId"), SubPoolStr) && !SubPoolStr.IsEmpty())
					{
						Entry.SubPoolId = FName(*SubPoolStr);
					}

					const TArray<TSharedPtr<FJsonValue>>* SlotsArr = nullptr;
					if ((*EObj)->TryGetArrayField(TEXT("Slots"), SlotsArr) && SlotsArr)
					{
						for (const TSharedPtr<FJsonValue>& SVal : *SlotsArr)
						{
							if (SVal.IsValid())
							{
								Entry.Slots.Add(static_cast<int32>(SVal->AsNumber()));
							}
						}
					}

					Pool.Entries.Add(Entry);
				}
			}

			Pools.Add(FName(*Pair.Key), Pool);
		}
	}

	UE_LOG(LogLootGenerator, Log, TEXT("LootRules 已加载: QualityPresets=%d, Pools=%d"), QualityPresets.Num(), Pools.Num());
	return true;
}

bool ULootGeneratorSubsystem::LoadMonsterProfilesJson(const FString& FilePath)
{
	Profiles.Empty();
	DropIdMap.Empty();

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		UE_LOG(LogLootGenerator, Error, TEXT("读取 MonsterLootProfiles.json 失败: %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogLootGenerator, Error, TEXT("解析 MonsterLootProfiles.json 失败: %s"), *FilePath);
		return false;
	}

	const TSharedPtr<FJsonObject>* ProfilesObj = nullptr;
	if (Root->TryGetObjectField(TEXT("Profiles"), ProfilesObj) && ProfilesObj && ProfilesObj->IsValid())
	{
		for (const auto& Pair : (*ProfilesObj)->Values)
		{
			const TSharedPtr<FJsonObject>* Obj = nullptr;
			if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(Obj) || !Obj || !Obj->IsValid()) continue;

			FMonsterLootProfileConfig Profile;
			FString TierStr;
			if ((*Obj)->TryGetStringField(TEXT("Tier"), TierStr))
			{
				Profile.Tier = ParseTierString(TierStr);
			}

			double Num = 0.0;
			if ((*Obj)->TryGetNumberField(TEXT("GoldChance"), Num)) Profile.GoldChance = FMath::Clamp(static_cast<float>(Num), 0.0f, 1.0f);
			if ((*Obj)->TryGetNumberField(TEXT("GoldScaleMin"), Num)) Profile.GoldScaleMin = static_cast<float>(Num);
			if ((*Obj)->TryGetNumberField(TEXT("GoldScaleMax"), Num)) Profile.GoldScaleMax = static_cast<float>(Num);
			if ((*Obj)->TryGetNumberField(TEXT("UniqueWeightMultiplier"), Num)) Profile.UniqueWeightMultiplier = FMath::Max(0.0f, static_cast<float>(Num));
			if ((*Obj)->TryGetNumberField(TEXT("RareWeightMultiplier"), Num)) Profile.RareWeightMultiplier = FMath::Max(0.0f, static_cast<float>(Num));

			const TArray<TSharedPtr<FJsonValue>>* GuaranteedArr = nullptr;
			if ((*Obj)->TryGetArrayField(TEXT("GuaranteedDrops"), GuaranteedArr) && GuaranteedArr)
			{
				for (const TSharedPtr<FJsonValue>& GVal : *GuaranteedArr)
				{
					const TSharedPtr<FJsonObject>* GObj = nullptr;
					if (!GVal.IsValid() || !GVal->TryGetObject(GObj) || !GObj || !GObj->IsValid()) continue;

					FGuaranteedDropConfig G;
					if ((*GObj)->TryGetNumberField(TEXT("ItemID"), Num)) G.ItemID = static_cast<int32>(Num);
					if ((*GObj)->TryGetNumberField(TEXT("ForcedQuality"), Num)) G.ForcedQuality = static_cast<int32>(Num);
					if ((*GObj)->TryGetNumberField(TEXT("Count"), Num)) G.Count = FMath::Max(1, static_cast<int32>(Num));
					(*GObj)->TryGetBoolField(TEXT("bFirstKillOnly"), G.bFirstKillOnly);
					Profile.GuaranteedDrops.Add(G);
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* RollGroupsArr = nullptr;
			if ((*Obj)->TryGetArrayField(TEXT("RollGroups"), RollGroupsArr) && RollGroupsArr)
			{
				for (const TSharedPtr<FJsonValue>& RVal : *RollGroupsArr)
				{
					const TSharedPtr<FJsonObject>* RObj = nullptr;
					if (!RVal.IsValid() || !RVal->TryGetObject(RObj) || !RObj || !RObj->IsValid()) continue;

					FLootRollGroupConfig RG;
					FString PoolStr;
					if ((*RObj)->TryGetStringField(TEXT("PoolId"), PoolStr)) RG.PoolId = FName(*PoolStr);
					if ((*RObj)->TryGetNumberField(TEXT("RollCountMin"), Num)) RG.RollCountMin = FMath::Max(0, static_cast<int32>(Num));
					if ((*RObj)->TryGetNumberField(TEXT("RollCountMax"), Num)) RG.RollCountMax = FMath::Max(RG.RollCountMin, static_cast<int32>(Num));
					(*RObj)->TryGetBoolField(TEXT("bAllowDuplicate"), RG.bAllowDuplicate);
					Profile.RollGroups.Add(RG);
				}
			}

			Profiles.Add(FName(*Pair.Key), Profile);
		}
	}

	const TSharedPtr<FJsonObject>* DropMapObj = nullptr;
	if (Root->TryGetObjectField(TEXT("DropIdMap"), DropMapObj) && DropMapObj && DropMapObj->IsValid())
	{
		for (const auto& Pair : (*DropMapObj)->Values)
		{
			const int32 DropId = FCString::Atoi(*Pair.Key);
			FString ProfileName;
			if (DropId > 0 && Pair.Value.IsValid() && Pair.Value->TryGetString(ProfileName) && !ProfileName.IsEmpty())
			{
				DropIdMap.Add(DropId, FName(*ProfileName));
			}
		}
	}

	UE_LOG(LogLootGenerator, Log, TEXT("MonsterLootProfiles 已加载: Profiles=%d, DropIdMap=%d"), Profiles.Num(), DropIdMap.Num());
	return true;
}

TArray<FString> ULootGeneratorSubsystem::ValidateConfigs() const
{
	TArray<FString> Errors;
	const UItemDatabaseSubsystem* DB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;

	// 1. 校验池子引用与子池循环
	for (const auto& Pair : Pools)
	{
		for (const FLootPoolEntry& E : Pair.Value.Entries)
		{
			if (E.Type == ELootPoolEntryType::SubPool)
			{
				if (!Pools.Contains(E.SubPoolId))
				{
					Errors.Add(FString::Printf(TEXT("Pool '%s' 引用了不存在的子池 '%s'"), *Pair.Key.ToString(), *E.SubPoolId.ToString()));
				}
				else if (E.SubPoolId == Pair.Key)
				{
					Errors.Add(FString::Printf(TEXT("Pool '%s' 存在自循环引用"), *Pair.Key.ToString()));
				}
			}
			else if (E.Type == ELootPoolEntryType::FixedItem)
			{
				if (DB && !DB->Find(E.ItemID))
				{
					Errors.Add(FString::Printf(TEXT("Pool '%s' 引用了不存在的道具 ItemID=%d"), *Pair.Key.ToString(), E.ItemID));
				}
			}
		}
	}

	// 2. 校验 Profile 引用
	for (const auto& Pair : Profiles)
	{
		for (const FGuaranteedDropConfig& G : Pair.Value.GuaranteedDrops)
		{
			if (DB && !DB->Find(G.ItemID))
			{
				Errors.Add(FString::Printf(TEXT("Profile '%s' 必掉项引用了不存在的道具 ItemID=%d"), *Pair.Key.ToString(), G.ItemID));
			}
		}
		for (const FLootRollGroupConfig& RG : Pair.Value.RollGroups)
		{
			if (!Pools.Contains(RG.PoolId))
			{
				Errors.Add(FString::Printf(TEXT("Profile '%s' 引用了不存在的掉落池 '%s'"), *Pair.Key.ToString(), *RG.PoolId.ToString()));
			}
		}
	}

	// 3. 校验 DropIdMap
	for (const auto& Pair : DropIdMap)
	{
		if (!Profiles.Contains(Pair.Value))
		{
			Errors.Add(FString::Printf(TEXT("DropIdMap[%d] 指向了不存在的 Profile '%s'"), Pair.Key, *Pair.Value.ToString()));
		}
	}

	return Errors;
}

FName ULootGeneratorSubsystem::ResolveProfileId(FName ExplicitProfileId, int32 DropId, EMonsterLootTier FallbackTier) const
{
	if (!ExplicitProfileId.IsNone() && Profiles.Contains(ExplicitProfileId))
	{
		return ExplicitProfileId;
	}
	if (DropId > 0)
	{
		if (const FName* Mapped = DropIdMap.Find(DropId))
		{
			if (Profiles.Contains(*Mapped))
			{
				return *Mapped;
			}
		}
	}
	const FName DefaultId = GetDefaultProfileIdForTier(FallbackTier);
	if (Profiles.Contains(DefaultId))
	{
		return DefaultId;
	}
	return FName(TEXT("Default_Normal"));
}

int32 ULootGeneratorSubsystem::RollQualityForItem(const FMonsterLootProfileConfig& Profile, const FLootContext& Context, FRandomStream& Rng) const
{
	const FLootQualityPreset* PresetPtr = QualityPresets.Find(Profile.Tier);
	if (!PresetPtr)
	{
		PresetPtr = QualityPresets.Find(Context.MonsterTier);
	}
	const FLootQualityPreset Preset = PresetPtr ? *PresetPtr : FLootQualityPreset();

	// 1. 先按配置权重归一化为 MF=0 时的精确基础概率
	const float W0 = FMath::Max(0.0f, Preset.NormalWeight);
	const float W1 = FMath::Max(0.0f, Preset.MagicWeight);
	const float W2 = FMath::Max(0.0f, Preset.RareWeight * Profile.RareWeightMultiplier);
	const float W3 = FMath::Max(0.0f, Preset.UniqueWeight * Profile.UniqueWeightMultiplier);

	const float Total = W0 + W1 + W2 + W3;
	if (Total <= KINDA_SMALL_NUMBER) return 0;

	const float EffMagicMF = FLootMFConfig::ApplyDiminishingReturns(Context.MagicFind, MFConfig.MagicK);
	const float EffRareMF = FLootMFConfig::ApplyDiminishingReturns(Context.MagicFind, MFConfig.RareK);
	const float EffUniqueMF = FLootMFConfig::ApplyDiminishingReturns(Context.MagicFind, MFConfig.UniqueK);

	// 2. 暗黑2 经典“自上而下级联判定”：先判暗金 -> 再判稀有 -> 再判魔法 -> 否则普通
	//    保证 MF=0 时严格等于配置权重占比，且 MF>0 时任何阶级（含 NormalWeight=0 的 Boss）的高品质率均单调提升
	const float BaseP3 = W3 / Total;
	if (BaseP3 > 0.0f)
	{
		const float P3 = FMath::Min(0.85f, BaseP3 * (1.0f + EffUniqueMF / 100.0f));
		if (Rng.FRand() < P3) return 3;
	}

	const float RemAfterUnique = W0 + W1 + W2;
	const float BaseCondP2 = RemAfterUnique > KINDA_SMALL_NUMBER ? (W2 / RemAfterUnique) : 0.0f;
	if (BaseCondP2 > 0.0f)
	{
		const float P2 = FMath::Min(0.90f, BaseCondP2 * (1.0f + EffRareMF / 100.0f));
		if (Rng.FRand() < P2) return 2;
	}

	if (W0 <= KINDA_SMALL_NUMBER)
	{
		return 1; // 不出白装的 Boss/高阶怪，至少魔法蓝装起
	}

	const float RemAfterRare = W0 + W1;
	const float BaseCondP1 = RemAfterRare > KINDA_SMALL_NUMBER ? (W1 / RemAfterRare) : 0.0f;
	if (BaseCondP1 > 0.0f)
	{
		const float P1 = FMath::Min(0.95f, BaseCondP1 * (1.0f + EffMagicMF / 100.0f));
		if (Rng.FRand() < P1) return 1;
	}

	return 0;
}

int32 ULootGeneratorSubsystem::PickDynamicEquipItemId(
	const FLootPoolEntry& Entry, int32 MonsterLevel, const TSet<int32>& UsedItemIds, bool bAllowDuplicate, FRandomStream& Rng) const
{
	const UItemDatabaseSubsystem* DB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;
	if (!DB) return 0;

	const int32 MinLv = FMath::Max(0, MonsterLevel + Entry.LevelMinOffset);
	const int32 MaxLv = MonsterLevel + Entry.LevelMaxOffset;

	auto CollectCandidates = [&](bool bEnforceLevelWindow, bool bEnforceNoDuplicate) -> TArray<const FItemBaseRow*>
	{
		TArray<const FItemBaseRow*> Out;
		for (int32 ID : DB->GetOrderedIDs())
		{
			const FItemBaseRow* Row = DB->Find(ID);
			if (!Row || !Row->IsEquipment() || !Row->bCanWorldDrop || Row->DropWeight <= 0) continue;
			if (Entry.Slots.Num() > 0 && !Entry.Slots.Contains(Row->EquipType)) continue;
			if (bEnforceNoDuplicate && !bAllowDuplicate && UsedItemIds.Contains(ID)) continue;
			if (bEnforceLevelWindow && (Row->Level < MinLv || Row->Level > MaxLv)) continue;
			Out.Add(Row);
		}
		return Out;
	};

	// 1. 先按严格等级窗口筛选
	TArray<const FItemBaseRow*> Candidates = CollectCandidates(true, true);

	// 2. 若当前测试表该等级段空缺，则回退到最接近怪物等级的同槽位装备（确保任意等级测试都有合理掉落）
	if (Candidates.Num() == 0)
	{
		TArray<const FItemBaseRow*> AllSlotItems = CollectCandidates(false, true);
		if (AllSlotItems.Num() == 0)
		{
			AllSlotItems = CollectCandidates(false, false);
		}
		if (AllSlotItems.Num() > 0)
		{
			int32 BestDelta = INT_MAX;
			for (const FItemBaseRow* Row : AllSlotItems)
			{
				BestDelta = FMath::Min(BestDelta, FMath::Abs(Row->Level - MonsterLevel));
			}
			// 取最接近等级 ±6 级范围内的装备，避免总掉同一件
			for (const FItemBaseRow* Row : AllSlotItems)
			{
				if (FMath::Abs(Row->Level - MonsterLevel) <= BestDelta + 6)
				{
					Candidates.Add(Row);
				}
			}
		}
	}

	if (Candidates.Num() == 0) return 0;

	int32 TotalWeight = 0;
	for (const FItemBaseRow* Row : Candidates)
	{
		TotalWeight += FMath::Max(1, Row->DropWeight);
	}

	int32 Roll = Rng.RandRange(0, TotalWeight - 1);
	for (const FItemBaseRow* Row : Candidates)
	{
		Roll -= FMath::Max(1, Row->DropWeight);
		if (Roll < 0)
		{
			return Row->ID;
		}
	}
	return Candidates.Last()->ID;
}

void ULootGeneratorSubsystem::RollFromPool(
	FName PoolId,
	const FMonsterLootProfileConfig& Profile,
	const FLootContext& Context,
	TSet<int32>& UsedItemIds,
	bool bAllowDuplicate,
	int32 Depth,
	FRandomStream& Rng,
	TArray<FLootDropResult>& OutResults) const
{
	if (Depth > LootGeneratorInternal::MaxSubPoolDepth) return;
	const FLootPoolConfig* Pool = Pools.Find(PoolId);
	if (!Pool) return;

	float TotalWeight = FMath::Max(0.0f, Pool->NoDropWeight);
	for (const FLootPoolEntry& Entry : Pool->Entries)
	{
		TotalWeight += FMath::Max(0.0f, Entry.Weight);
	}
	if (TotalWeight <= KINDA_SMALL_NUMBER) return;

	float Roll = Rng.FRandRange(0.0f, TotalWeight);
	if (Roll < Pool->NoDropWeight)
	{
		return; // 空掉（NoDrop）
	}
	Roll -= Pool->NoDropWeight;

	const FLootPoolEntry* Chosen = nullptr;
	for (const FLootPoolEntry& Entry : Pool->Entries)
	{
		const float W = FMath::Max(0.0f, Entry.Weight);
		if (Roll < W)
		{
			Chosen = &Entry;
			break;
		}
		Roll -= W;
	}
	if (!Chosen && Pool->Entries.Num() > 0)
	{
		Chosen = &Pool->Entries.Last();
	}
	if (!Chosen) return;

	const UItemDatabaseSubsystem* DB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;
	if (!DB) return;

	switch (Chosen->Type)
	{
	case ELootPoolEntryType::SubPool:
		RollFromPool(Chosen->SubPoolId, Profile, Context, UsedItemIds, bAllowDuplicate, Depth + 1, Rng, OutResults);
		break;

	case ELootPoolEntryType::Gold:
	{
		const int32 Base = FMath::Max(1, Context.BaseGold);
		const float Scale = Rng.FRandRange(Chosen->GoldScaleMin, FMath::Max(Chosen->GoldScaleMin, Chosen->GoldScaleMax));
		const int32 Amount = FMath::Max(1, FMath::RoundToInt(Base * Scale));
		FLootDropResult Res;
		Res.bIsGold = true;
		Res.GoldAmount = Amount;
		OutResults.Add(Res);
		break;
	}

	case ELootPoolEntryType::FixedItem:
	{
		if (!DB->Find(Chosen->ItemID)) break;
		const int32 Quality = Chosen->ForcedQuality >= 0 ? Chosen->ForcedQuality : RollQualityForItem(Profile, Context, Rng);
		const FItemInstance Inst = DB->CreateRolledInstance(Chosen->ItemID, Quality, Rng);
		if (Inst.IsValid())
		{
			UsedItemIds.Add(Chosen->ItemID);
			FLootDropResult Res;
			Res.bIsGold = false;
			Res.Item = Inst;
			OutResults.Add(Res);
		}
		break;
	}

	case ELootPoolEntryType::DynamicEquip:
	{
		const int32 PickedID = PickDynamicEquipItemId(*Chosen, Context.MonsterLevel, UsedItemIds, bAllowDuplicate, Rng);
		if (PickedID <= 0) break;
		const int32 Quality = RollQualityForItem(Profile, Context, Rng);
		const FItemInstance Inst = DB->CreateRolledInstance(PickedID, Quality, Rng);
		if (Inst.IsValid())
		{
			UsedItemIds.Add(PickedID);
			FLootDropResult Res;
			Res.bIsGold = false;
			Res.Item = Inst;
			OutResults.Add(Res);
		}
		break;
	}
	}
}

bool ULootGeneratorSubsystem::RollLoot(FName ProfileId, const FLootContext& Context, TArray<FLootDropResult>& OutResults) const
{
	return RollLootWithSeed(ProfileId, Context, FMath::Rand(), OutResults);
}

bool ULootGeneratorSubsystem::RollLootWithSeed(FName ProfileId, const FLootContext& Context, int32 Seed, TArray<FLootDropResult>& OutResults) const
{
	OutResults.Reset();
	const FMonsterLootProfileConfig* Profile = Profiles.Find(ProfileId);
	if (!Profile)
	{
		const FName Fallback = GetDefaultProfileIdForTier(Context.MonsterTier);
		Profile = Profiles.Find(Fallback);
	}
	if (!Profile) return false;

	const UItemDatabaseSubsystem* DB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;
	if (!DB) return false;

	FRandomStream Rng(Seed);
	TSet<int32> UsedItemIds;

	// 1. 独立金币掉落
	if (Context.BaseGold > 0 && Profile->GoldChance > 0.0f && Rng.FRand() < Profile->GoldChance)
	{
		const float Scale = Rng.FRandRange(Profile->GoldScaleMin, FMath::Max(Profile->GoldScaleMin, Profile->GoldScaleMax));
		const int32 Amount = FMath::Max(1, FMath::RoundToInt(Context.BaseGold * Scale));
		FLootDropResult GoldRes;
		GoldRes.bIsGold = true;
		GoldRes.GoldAmount = Amount;
		OutResults.Add(GoldRes);
	}

	// 2. 必掉区（Profile 内必掉 + 怪物/关卡实例追加必掉）
	auto ProcessGuaranteed = [&](const TArray<FGuaranteedDropConfig>& List)
	{
		for (const FGuaranteedDropConfig& G : List)
		{
			if (G.bFirstKillOnly && !Context.bIsFirstKill) continue;
			if (!DB->Find(G.ItemID)) continue;
			const int32 SafeCount = FMath::Max(1, G.Count);
			for (int32 c = 0; c < SafeCount; ++c)
			{
				const int32 Quality = G.ForcedQuality >= 0 ? G.ForcedQuality : RollQualityForItem(*Profile, Context, Rng);
				const FItemInstance Inst = DB->CreateRolledInstance(G.ItemID, Quality, Rng);
				if (Inst.IsValid())
				{
					UsedItemIds.Add(G.ItemID);
					FLootDropResult Res;
					Res.bIsGold = false;
					Res.Item = Inst;
					Res.bFromGuaranteed = true;
					OutResults.Add(Res);
				}
			}
		}
	};
	ProcessGuaranteed(Profile->GuaranteedDrops);
	ProcessGuaranteed(Context.ExtraGuaranteedDrops);

	// 3. 随机掉落组
	for (const FLootRollGroupConfig& Group : Profile->RollGroups)
	{
		const int32 MinRolls = FMath::Max(0, Group.RollCountMin);
		const int32 MaxRolls = FMath::Max(MinRolls, Group.RollCountMax);
		const int32 RollCount = Rng.RandRange(MinRolls, MaxRolls);
		for (int32 r = 0; r < RollCount; ++r)
		{
			RollFromPool(Group.PoolId, *Profile, Context, UsedItemIds, Group.bAllowDuplicate, 0, Rng, OutResults);
		}
	}

	return true;
}

FLootSimulationReport ULootGeneratorSubsystem::SimulateProfile(FName ProfileId, int32 KillCount, const FLootContext& Context, int32 Seed) const
{
	FLootSimulationReport Report;
	Report.ProfileId = ProfileId;
	Report.KillCount = FMath::Max(1, KillCount);
	Report.QualityCounts.Init(0, 5);

	const UItemDatabaseSubsystem* DB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;
	TArray<FLootDropResult> SingleResults;

	for (int32 i = 0; i < Report.KillCount; ++i)
	{
		FLootContext IterCtx = Context;
		IterCtx.bIsFirstKill = (i == 0 && Context.bIsFirstKill);
		SingleResults.Reset();
		RollLootWithSeed(ProfileId, IterCtx, Seed + i * 7919, SingleResults);

		for (const FLootDropResult& R : SingleResults)
		{
			if (R.bIsGold)
			{
				Report.TotalGold += R.GoldAmount;
			}
			else if (R.Item.IsValid())
			{
				Report.TotalItems++;
				const int32 Q = FMath::Clamp(R.Item.Quality, 0, 4);
				Report.QualityCounts[Q]++;
				Report.ItemCounts.FindOrAdd(R.Item.BaseID)++;
			}
		}
	}

	const float AvgItems = static_cast<float>(Report.TotalItems) / Report.KillCount;
	const float AvgGold = static_cast<float>(Report.TotalGold) / Report.KillCount;
	auto Pct = [&](int32 Q) -> float
	{
		return Report.TotalItems > 0 ? (100.0f * Report.QualityCounts[Q] / Report.TotalItems) : 0.0f;
	};

	FString Summary = FString::Printf(
		TEXT("[LootSim] Profile=%s | Kills=%d | Lv=%d | MF=%.0f%%\n")
		TEXT("  平均掉落: %.2f 件/次 | 平均金币: %.1f/次 | 总装备数: %d\n")
		TEXT("  品质分布: 白 %.1f%% (%d) | 蓝 %.1f%% (%d) | 黄 %.1f%% (%d) | 暗金 %.1f%% (%d)"),
		*ProfileId.ToString(), Report.KillCount, Context.MonsterLevel, Context.MagicFind,
		AvgItems, AvgGold, Report.TotalItems,
		Pct(0), Report.QualityCounts[0],
		Pct(1), Report.QualityCounts[1],
		Pct(2), Report.QualityCounts[2],
		Pct(3), Report.QualityCounts[3]);

	// 附加掉落次数最多的前 8 件道具
	TArray<TPair<int32, int32>> TopItems;
	for (const auto& Pair : Report.ItemCounts)
	{
		TopItems.Add(Pair);
	}
	TopItems.Sort([](const TPair<int32, int32>& A, const TPair<int32, int32>& B) { return A.Value > B.Value; });

	const int32 ShowNum = FMath::Min(8, TopItems.Num());
	if (ShowNum > 0)
	{
		Summary += TEXT("\n  主要产出:");
		for (int32 i = 0; i < ShowNum; ++i)
		{
			const int32 ItemID = TopItems[i].Key;
			const int32 Cnt = TopItems[i].Value;
			const FItemBaseRow* Row = DB ? DB->Find(ItemID) : nullptr;
			const FString Name = Row ? Row->Name : FString::FromInt(ItemID);
			Summary += FString::Printf(TEXT(" [%d %s: %d次(%.1f%%)]"), ItemID, *Name, Cnt, 100.0f * Cnt / Report.KillCount);
		}
	}

	Report.FormattedSummary = Summary;
	return Report;
}

void ULootGeneratorSubsystem::RegisterConsoleCommands()
{
	UnregisterConsoleCommands();

	TWeakObjectPtr<ULootGeneratorSubsystem> WeakThis(this);

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Loot.Reload"),
		TEXT("热重载 ItemBase.csv / RandomAffixes.json / LootRules.json / MonsterLootProfiles.json"),
		FConsoleCommandDelegate::CreateLambda([WeakThis]()
		{
			if (ULootGeneratorSubsystem* Self = WeakThis.Get())
			{
				Self->ReloadAllLootConfigs();
			}
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Loot.Sim"),
		TEXT("模拟击杀掉落: Loot.Sim <ProfileId> [KillCount=1000] [MonsterLevel=10] [MF=0] [BaseGold=100]"),
		FConsoleCommandWithArgsDelegate::CreateLambda([WeakThis](const TArray<FString>& Args)
		{
			ULootGeneratorSubsystem* Self = WeakThis.Get();
			if (!Self) return;

			const FName ProfileId = Args.Num() >= 1 ? FName(*Args[0]) : FName(TEXT("Boss_SkeletonKing"));
			const int32 Kills = Args.Num() >= 2 ? FMath::Clamp(FCString::Atoi(*Args[1]), 1, 100000) : 1000;
			const int32 Lvl = Args.Num() >= 3 ? FCString::Atoi(*Args[2]) : 35;
			const float MF = Args.Num() >= 4 ? FCString::Atof(*Args[3]) : 0.0f;
			const int32 BaseGold = Args.Num() >= 5 ? FCString::Atoi(*Args[4]) : 1000;

			FLootContext Ctx;
			Ctx.MonsterLevel = Lvl;
			Ctx.MagicFind = MF;
			Ctx.BaseGold = BaseGold;

			const FLootSimulationReport Report = Self->SimulateProfile(ProfileId, Kills, Ctx);
			UE_LOG(LogLootGenerator, Display, TEXT("%s"), *Report.FormattedSummary);
		}),
		ECVF_Default));
}

void ULootGeneratorSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleObject* Cmd : ConsoleCommands)
	{
		if (Cmd)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Cmd);
		}
	}
	ConsoleCommands.Empty();
}
