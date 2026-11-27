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

/** 道具品质（对应暗黑2与旧项目 quality 枚举） */
UENUM(BlueprintType)
enum class EItemQuality : uint8
{
	Normal = 0 UMETA(DisplayName = "普通"),
	Magic = 1 UMETA(DisplayName = "魔法"),
	Rare = 2 UMETA(DisplayName = "稀有"),
	Unique = 3 UMETA(DisplayName = "暗金"),
	Set = 4 UMETA(DisplayName = "套装")
};

/** 随机词缀属性 ID（与旧项目 AttributeEnum 保持一致，AffixKey = EquipType * 1000 + AttrID） */
UENUM(BlueprintType)
enum class EItemAffixAttr : uint8
{
	None = 0 UMETA(Hidden),
	HpMax = 1 UMETA(DisplayName = "生命"),
	MpMax = 2 UMETA(DisplayName = "法力/体力"),
	AttackLow = 3 UMETA(DisplayName = "最小攻击力"),
	AttackHigh = 4 UMETA(DisplayName = "最大攻击力"),
	Defence = 5 UMETA(DisplayName = "防御力"),
	Critical = 7 UMETA(DisplayName = "暴击"),
	Block = 8 UMETA(DisplayName = "格挡"),
	Proficient = 9 UMETA(DisplayName = "熟练"),
	SwordHarm = 10 UMETA(DisplayName = "单手武器伤害"),
	SpearHarm = 11 UMETA(DisplayName = "长柄武器伤害"),
	BowHarm = 12 UMETA(DisplayName = "远程武器伤害"),
	PoisonDefence = 13 UMETA(DisplayName = "剧毒抗性"),
	ElementPoison = 14 UMETA(DisplayName = "毒素伤害"),
	ElementFire = 15 UMETA(DisplayName = "火焰伤害"),
	ElementIce = 16 UMETA(DisplayName = "寒冰伤害"),
	MoreMoney = 17 UMETA(DisplayName = "获得更多金币"),
	Puncture = 18 UMETA(DisplayName = "穿刺"),
	Stun = 19 UMETA(DisplayName = "击晕"),
	AttackPercent = 20 UMETA(DisplayName = "攻击强化"),
	DefencePercent = 21 UMETA(DisplayName = "防御强化"),
	BlockPercent = 22 UMETA(DisplayName = "格挡强化"),
	FireDefence = 23 UMETA(DisplayName = "火焰抗性"),
	IceDefence = 24 UMETA(DisplayName = "寒冰抗性")
};

/** 随机词缀配置行（对应旧项目 RandomPropBaseVo，数据放在 Content/Data/RandomAffixes.json） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FRandomAffixRow
{
	GENERATED_BODY()

	/** 唯一键 = EquipType * 1000 + AttrID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	bool bEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	bool bPercentData = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FString PreName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FString LastName;

	/** 9 档等级区间（0, 10, 20, ..., 80 级）的最小值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	TArray<int32> MinValues;

	/** 9 档等级区间（0, 10, 20, ..., 80 级）的最大值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	TArray<int32> MaxValues;

	int32 GetEquipType() const { return ID / 1000; }
	int32 GetAttrID() const { return ID % 1000; }
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

	/** 最低品质（0=白，1=蓝，2=黄，3=暗金/橙） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Loot")
	int32 QualityMin = 0;

	/** 暗金/高品质随机词缀数值缩放百分比（默认 100，暗金常用 150） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Loot")
	int32 HiQualityScale = 100;

	/** 固定额外属性列表（X = AttrID，Y = Value），暗金或特殊装备自带 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Loot")
	TArray<FIntPoint> ExtraAttributes;

	/** 动态装备池内的相对掉落权重（默认 100） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Loot")
	int32 DropWeight = 100;

	/** 是否允许进入通用动态装备池（Boss 专属暗金/任务品填 false） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Loot")
	bool bCanWorldDrop = true;

	FIntPoint GetCellSize() const { return FIntPoint(FMath::Max(1, CellSizeX), FMath::Max(1, CellSizeY)); }
	bool IsWeapon() const { return EquipType == 7 || EquipType == 9 || EquipType == 10; }
	bool IsEquipment() const { return EquipType > 0; }
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Count = 1;

	/** 随机词缀与额外属性（Key = EquipType * 1000 + AttrID, Value = 数值） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TMap<int32, int32> DynamicAttributes;

	bool IsValid() const { return BaseID > 0 && InstanceId.IsValid(); }
	bool HasAffixes() const { return DynamicAttributes.Num() > 0; }
	int32 GetDynamicAttr(int32 EquipType, int32 AttrID) const
	{
		const int32* Found = DynamicAttributes.Find(EquipType * 1000 + AttrID);
		return Found ? *Found : 0;
	}
	void Reset() { *this = FItemInstance(); }
};
