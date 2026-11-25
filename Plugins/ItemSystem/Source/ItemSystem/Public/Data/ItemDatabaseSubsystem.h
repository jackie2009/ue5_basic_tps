#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ItemSystemTypes.h"
#include "ItemDatabaseSubsystem.generated.h"

class UTexture2D;

/**
 * 道具配置库：读取插件 Content/Data/ItemBase.csv，按 ID 查询配置与图标。
 * 独立于项目的 TableDataManagerSubsystem，保证插件可以整体拆走。
 */
UCLASS()
class ITEMSYSTEM_API UItemDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UItemDatabaseSubsystem* Get(const UObject* WorldContext);

	const FItemBaseRow* Find(int32 BaseID) const { return Rows.Find(BaseID); }
	const TMap<int32, FItemBaseRow>& GetAllRows() const { return Rows; }

	/** 按配置顺序排列的 ID 列表 */
	const TArray<int32>& GetOrderedIDs() const { return OrderedIDs; }

	FIntPoint GetCellSize(int32 BaseID) const;
	FIntPoint GetCellSize(const FItemInstance& Item) const { return GetCellSize(Item.BaseID); }

	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	UTexture2D* GetIcon(int32 BaseID);

	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	FItemInstance CreateInstance(int32 BaseID) const;

private:
	void LoadCsv(const FString& FilePath);

	TMap<int32, FItemBaseRow> Rows;
	TArray<int32> OrderedIDs;
	FString ContentDir;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UTexture2D>> IconCache;
};
