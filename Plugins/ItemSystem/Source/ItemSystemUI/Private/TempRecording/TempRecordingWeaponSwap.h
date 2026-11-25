#pragma once

// ============================================================================
// 【临时 / 非正式】录像用：装备栏单手武器换成 201 → 英雄拿 BP_Sword，2010 → BP_Axe。
// 通过反射调用 BP_Hero 现有的 Equip / UpdateCurrentWeapon，不改任何蓝图，也不编译期依赖战斗模块。
// 删除方法：删掉 TempRecording 目录，并去掉 ItemSystemUISubsystem.cpp 里标了 [TempRecording] 的几行。
// ============================================================================

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TempRecordingWeaponSwap.generated.h"

class UEquipmentComponent;
class APlayerController;

UCLASS()
class UTempRecordingWeaponSwap : public UObject
{
	GENERATED_BODY()

public:
	void Bind(UEquipmentComponent* InEquipment, APlayerController* InPC);

private:
	UFUNCTION()
	void HandleEquipmentChanged();

	void SwapHeroMeleeWeapon(UClass* NewClass);

	UPROPERTY()
	TObjectPtr<UEquipmentComponent> Equipment;

	TWeakObjectPtr<APlayerController> PC;
	int32 LastAppliedID = 0;
};
