#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryGridMath.h"
#include "InventoryGridComponent.generated.h"

class UItemDatabaseSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoldChanged, int32, NewGold, int32, Delta);

USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Anchor = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FItemInstance Item;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FIntPoint Size = FIntPoint(1, 1);
};

/** 网格背包（对应 Unity BagModel.bagItems + BagLogic_Grid）。只管数据与规则，不含任何 UI。 */
UCLASS(ClassGroup = (ItemSystem), meta = (BlueprintSpawnableComponent))
class ITEMSYSTEM_API UInventoryGridComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryGridComponent();

	/** 背包列数（Unity BagGridWidth = 9） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Width = 9;

	/** 背包行数（Unity 背包 81 格 = 9 行） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Height = 9;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnGoldChanged OnGoldChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Gold = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddGold(int32 Amount);

	/** 放到第一个能放下的位置；返回锚点，-1 = 背包满 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(const FItemInstance& Item);

	/** 拿起覆盖该格的物品 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool PickUpAtCell(int32 Cell, FItemInstance& OutItem, int32& OutAnchor);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	EInventoryPlacement QueryPlace(const FItemInstance& Item, int32 TargetAnchor, int32& OutOverlapAnchor) const;

	/** 以 TargetAnchor 为左上角放下；若恰好压住一件，则把它取出到 OutSwapped */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool PlaceAt(const FItemInstance& Item, int32 TargetAnchor, FItemInstance& OutSwapped);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool FindItemAtCell(int32 Cell, FItemInstance& OutItem, int32& OutAnchor) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventoryEntry> GetEntries() const;

	FIntPoint GetItemSize(const FItemInstance& Item) const;
	int32 FindFirstFit(FIntPoint Size) const;

protected:
	virtual void InitializeComponent() override;

private:
	void EnsureSlots();
	UItemDatabaseSubsystem* GetDatabase() const;

	UPROPERTY()
	TArray<FItemInstance> Slots;
};
