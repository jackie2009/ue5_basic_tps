#include "TempRecording/TempRecordingWeaponSwap.h"

#include "Equipment/EquipmentComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogTempWeaponSwap, Log, All);

namespace TempWeaponSwap
{
	// 道具 ID → 英雄武器蓝图
	UClass* ClassForItem(int32 BaseID)
	{
		const TCHAR* Path = nullptr;
		if (BaseID == 201) Path = TEXT("/Game/Items/AdvanceWeaponPack/Blueprints/BP_Sword.BP_Sword_C");
		if (BaseID == 2010) Path = TEXT("/Game/Items/AdvanceWeaponPack/Blueprints/BP_Axe.BP_Axe_C");
		return Path ? LoadClass<AActor>(nullptr, Path) : nullptr;
	}

	bool IsMeleeWeaponActor(const UObject* Obj)
	{
		if (!Obj) return false;
		const FString Name = Obj->GetClass()->GetName();
		return Name.StartsWith(TEXT("BP_Sword")) || Name.StartsWith(TEXT("BP_Axe"));
	}

	int64 ReadInt(const FProperty* Prop, const void* Ptr)
	{
		if (const FEnumProperty* EP = CastField<FEnumProperty>(Prop))
			return EP->GetUnderlyingProperty()->GetSignedIntPropertyValue(Ptr);
		if (const FNumericProperty* NP = CastField<FNumericProperty>(Prop))
			return NP->GetSignedIntPropertyValue(Ptr);
		return -1;
	}

	void WriteInt(const FProperty* Prop, void* Ptr, int64 Value)
	{
		if (const FEnumProperty* EP = CastField<FEnumProperty>(Prop))
			EP->GetUnderlyingProperty()->SetIntPropertyValue(Ptr, Value);
		else if (const FNumericProperty* NP = CastField<FNumericProperty>(Prop))
			NP->SetIntPropertyValue(Ptr, Value);
	}

	/** 调用蓝图函数：类参数填 ClassArg，整数/枚举参数填 IntArg */
	bool CallBP(UObject* Target, FName FuncName, UClass* ClassArg, int64 IntArg)
	{
		UFunction* Func = Target ? Target->FindFunction(FuncName) : nullptr;
		if (!Func) return false;
		uint8* Params = static_cast<uint8*>(FMemory_Alloca(FMath::Max<int32>(1, Func->ParmsSize)));
		FMemory::Memzero(Params, FMath::Max<int32>(1, Func->ParmsSize));
		for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->InitializeValue_InContainer(Params);
			if (It->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm)) continue;
			void* ValuePtr = It->ContainerPtrToValuePtr<void>(Params);
			if (const FClassProperty* CP = CastField<FClassProperty>(*It))
				CP->SetObjectPropertyValue(ValuePtr, ClassArg);
			else
				WriteInt(*It, ValuePtr, IntArg);
		}
		Target->ProcessEvent(Func, Params);
		for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->DestroyValue_InContainer(Params);
		}
		return true;
	}
}

void UTempRecordingWeaponSwap::Bind(UEquipmentComponent* InEquipment, APlayerController* InPC)
{
	Equipment = InEquipment;
	PC = InPC;
	if (Equipment)
	{
		LastAppliedID = Equipment->GetItem(EEquipSlot::MainHand).BaseID; // 初始 201 = 英雄本来就拿着剑
		Equipment->OnEquipmentChanged.AddUniqueDynamic(this, &UTempRecordingWeaponSwap::HandleEquipmentChanged);
	}
}

void UTempRecordingWeaponSwap::HandleEquipmentChanged()
{
	const int32 ID = Equipment ? Equipment->GetItem(EEquipSlot::MainHand).BaseID : 0;
	if (ID == LastAppliedID) return;
	if (UClass* NewClass = TempWeaponSwap::ClassForItem(ID))
	{
		LastAppliedID = ID;
		SwapHeroMeleeWeapon(NewClass);
	}
}

void UTempRecordingWeaponSwap::SwapHeroMeleeWeapon(UClass* NewClass)
{
	APawn* Hero = PC.IsValid() ? PC->GetPawn() : nullptr;
	FMapProperty* MapProp = Hero ? FindFProperty<FMapProperty>(Hero->GetClass(), TEXT("EquippedWeapons")) : nullptr;
	if (!MapProp)
	{
		UE_LOG(LogTempWeaponSwap, Warning, TEXT("英雄没有 EquippedWeapons，跳过"));
		return;
	}

	// 1. 找到当前那把剑/斧及其类型、所挂插槽
	FScriptMapHelper Map(MapProp, MapProp->ContainerPtrToValuePtr<void>(Hero));
	int64 WeaponType = -1;
	AActor* OldWeapon = nullptr;
	for (FScriptMapHelper::FIterator It(Map); It; ++It)
	{
		UObject* Value = CastField<FObjectPropertyBase>(Map.GetValueProperty())->GetObjectPropertyValue(Map.GetValuePtr(It));
		if (TempWeaponSwap::IsMeleeWeaponActor(Value))
		{
			WeaponType = TempWeaponSwap::ReadInt(Map.GetKeyProperty(), Map.GetKeyPtr(It));
			OldWeapon = Cast<AActor>(Value);
			break;
		}
	}
	if (!OldWeapon || WeaponType < 0)
	{
		UE_LOG(LogTempWeaponSwap, Warning, TEXT("没找到英雄的近战武器，跳过"));
		return;
	}
	if (OldWeapon->GetClass() == NewClass) return;

	USceneComponent* AttachParent = OldWeapon->GetRootComponent() ? OldWeapon->GetRootComponent()->GetAttachParent() : nullptr;
	const FName AttachSocket = OldWeapon->GetAttachParentSocketName();

	// 2. 调用英雄自己的 Equip(weaponClass, equipType)：生成新武器并写入 EquippedWeapons
	if (!TempWeaponSwap::CallBP(Hero, TEXT("Equip"), NewClass, WeaponType))
	{
		UE_LOG(LogTempWeaponSwap, Warning, TEXT("英雄没有 Equip 函数，跳过"));
		return;
	}

	// 3. 新武器挂到旧武器原来的位置（手上或收刀位），销毁旧武器
	AActor* NewWeapon = nullptr;
	for (FScriptMapHelper::FIterator It(Map); It; ++It)
	{
		if (TempWeaponSwap::ReadInt(Map.GetKeyProperty(), Map.GetKeyPtr(It)) == WeaponType)
		{
			NewWeapon = Cast<AActor>(CastField<FObjectPropertyBase>(Map.GetValueProperty())->GetObjectPropertyValue(Map.GetValuePtr(It)));
		}
	}
	if (NewWeapon && NewWeapon != OldWeapon && AttachParent)
	{
		NewWeapon->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
	}
	if (NewWeapon != OldWeapon)
	{
		OldWeapon->Destroy();
	}

	// 4. 如果换的是当前手上的武器，刷新英雄的当前武器引用
	if (const FProperty* CurProp = FindFProperty<FProperty>(Hero->GetClass(), TEXT("CurrentWeaponType")))
	{
		if (TempWeaponSwap::ReadInt(CurProp, CurProp->ContainerPtrToValuePtr<void>(Hero)) == WeaponType)
		{
			TempWeaponSwap::CallBP(Hero, TEXT("UpdateCurrentWeapon"), nullptr, WeaponType);
		}
	}
	UE_LOG(LogTempWeaponSwap, Log, TEXT("英雄近战武器已换成 %s"), *NewClass->GetName());
}
