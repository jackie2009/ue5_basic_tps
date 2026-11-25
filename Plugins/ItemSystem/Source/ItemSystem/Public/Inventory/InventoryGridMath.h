#pragma once

#include "CoreMinimal.h"
#include "Data/ItemSystemTypes.h"
#include "InventoryGridMath.generated.h"

/** 放置结果：空位 / 交换（恰好压住一件）/ 不可放 */
UENUM(BlueprintType)
enum class EInventoryPlacement : uint8
{
	Blocked,
	Empty,
	Swap
};

/**
 * 网格占格算法（移植自 Unity InventoryCellHelper）。
 * 容器是 Width*Height 的数组，物品只存放在左上角锚点格，其余被占的格子为空实例。
 */
struct ITEMSYSTEM_API FInventoryGridMath
{
	using FSizeFn = TFunctionRef<FIntPoint(const FItemInstance&)>;

	/** 返回每个格子被哪个锚点占用（-1 = 空） */
	static TArray<int32> BuildAnchorMap(const TArray<FItemInstance>& Slots, int32 Width, int32 Height, FSizeFn SizeOf);

	static bool FitsInside(int32 X, int32 Y, FIntPoint Size, int32 Width, int32 Height);

	/** 物品以 TargetAnchor 为左上角放下时的结果；Swap 时 OutOverlapAnchor 为被压住那件的锚点 */
	static EInventoryPlacement QueryPlacement(const TArray<FItemInstance>& Slots, int32 Width, int32 Height, FSizeFn SizeOf,
		int32 TargetAnchor, FIntPoint Size, int32& OutOverlapAnchor);

	/** 从左上开始找第一个能放下的空位（-1 = 放不下） */
	static int32 FindFirstFit(const TArray<FItemInstance>& Slots, int32 Width, int32 Height, FSizeFn SizeOf, FIntPoint Size);
};
