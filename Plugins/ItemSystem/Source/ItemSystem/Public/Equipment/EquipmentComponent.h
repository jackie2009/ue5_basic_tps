#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemSystemTypes.h"
#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

/** 装备栏（对应 Unity ABLT_Equipment.items + PropLogic 的穿戴规则）。不含战斗逻辑。 */
UCLASS(ClassGroup = (ItemSystem), meta = (BlueprintSpawnableComponent))
class ITEMSYSTEM_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();

	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

	/** 道具佩戴类型能否放入该槽（戒指可放两个戒指位） */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static bool CanEquipToSlot(int32 EquipType, EEquipSlot Slot);

	/** 道具的佩戴类型（0 = 不能装备） */
	int32 GetEquipType(const FItemInstance& Item) const;

	/** 自动选槽：戒指优先空的那个戒指位（同 Unity EquipOnAndPickupReplaced） */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	EEquipSlot ResolveSlot(const FItemInstance& Item) const;

	/** 穿到指定槽，原有装备放进 OutReplaced */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool Equip(const FItemInstance& Item, EEquipSlot Slot, FItemInstance& OutReplaced);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool Unequip(EEquipSlot Slot, FItemInstance& OutItem);

	UFUNCTION(BlueprintPure, Category = "Equipment")
	FItemInstance GetItem(EEquipSlot Slot) const;

	const TArray<FItemInstance>& GetAllSlots() const { return Slots; }

private:
	UPROPERTY()
	TArray<FItemInstance> Slots;
};
