#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemSystemTypes.generated.h"

/** 装备槽位。数值与 Unity PropEquipEnum 保持一致（Finger2 = 第二个戒指位）。 */
UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
	None = 0 UMETA(Hidden),
	Head = 1 UMETA(DisplayName = "头盔"),
	Body = 2 UMETA(DisplayName = "衣服"),
	Wrist = 3 UMETA(DisplayName = "手套"),
	Feet = 4 UMETA(DisplayName = "鞋子"),
	Finger1 = 5 UMETA(DisplayName = "戒指1"),
	Finger2 = 6 UMETA(DisplayName = "戒指2"),
	MainHand = 7 UMETA(DisplayName = "单手武器"),
	Neck = 8 UMETA(DisplayName = "项链"),
	MainHand2 = 9 UMETA(DisplayName = "长柄武器"),
	MainHand3 = 10 UMETA(DisplayName = "远程武器"),
	SubHand = 11 UMETA(DisplayName = "盾牌"),
	Belt = 12 UMETA(DisplayName = "腰带"),
	MAX UMETA(Hidden)
};

/** 道具配置行（对应 Unity PropBaseVo 的子集）。可直接作为 DataTable 行结构导入同一份 CSV。 */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FItemBaseRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FString Name;

	/** 佩戴类型，数值同 EEquipSlot（戒指为 5，可放 Finger1/Finger2）；0 = 非装备 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 EquipType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Level = 0;

	/** 防御 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Ac = 0;

	/** 最低攻击 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Dc = 0;

	/** 最高攻击 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Dc2 = 0;

	/** 格挡 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Block = 0;

	/** 背包占格宽 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 CellSizeX = 1;

	/** 背包占格高 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 CellSizeY = 1;

	/** 图标路径（相对插件 Content 目录的 PNG）。以后改为导入的贴图资源时，换成 TSoftObjectPtr<UTexture2D>。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FString Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FString Desc;

	FIntPoint GetCellSize() const { return FIntPoint(FMath::Max(1, CellSizeX), FMath::Max(1, CellSizeY)); }
	bool IsWeapon() const { return EquipType == 7 || EquipType == 9 || EquipType == 10; }
};

/** 道具实例（对应 Unity PropInstVo 的子集）。值类型，方便存档。 */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGuid InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 BaseID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Quality = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Durability = 0;

	/** 预留：随机词缀（Unity dynamicAttributes），当前不使用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TMap<int32, int32> DynamicAttributes;

	bool IsValid() const { return BaseID > 0 && InstanceId.IsValid(); }
	void Reset() { *this = FItemInstance(); }
};
