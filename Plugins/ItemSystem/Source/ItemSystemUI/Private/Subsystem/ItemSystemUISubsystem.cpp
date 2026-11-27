#include "Subsystem/ItemSystemUISubsystem.h"

#include "CodeLayout/CodeLayoutWidgets.h"
#include "Data/ItemDatabaseSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Equipment/EquipmentComponent.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/InventoryGridComponent.h"
#include "Subsystem/ItemSystemUISettings.h"
#include "UnrealClient.h"
#include "Widgets/InventoryScreenWidget.h"
#include "TempRecording/TempRecordingWeaponSwap.h" // [TempRecording]

/** Slate 输入预处理：不依赖项目的 Enhanced Input 配置，也不用改角色蓝图 */
class FItemSystemInputProcessor : public IInputProcessor
{
public:
	explicit FItemSystemInputProcessor(UItemSystemUISubsystem* InOwner) : Owner(InOwner) {}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		UItemSystemUISubsystem* S = Owner.Get();
		return S && S->HandleKeyDown(InKeyEvent.GetKey(), InKeyEvent.IsRepeat());
	}

private:
	TWeakObjectPtr<UItemSystemUISubsystem> Owner;
};

void UItemSystemUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FItemSystemInputProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
}

void UItemSystemUISubsystem::Deinitialize()
{
	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
	InputProcessor.Reset();
	if (Screen)
	{
		Screen->RemoveFromParent();
		Screen = nullptr;
	}
	Super::Deinitialize();
}

APlayerController* UItemSystemUISubsystem::GetPlayerController() const
{
	const ULocalPlayer* LP = GetLocalPlayer();
	return (LP && LP->GetWorld()) ? LP->GetPlayerController(LP->GetWorld()) : nullptr;
}

bool UItemSystemUISubsystem::IsGameViewportFocused() const
{
	const ULocalPlayer* LP = GetLocalPlayer();
	return LP && LP->ViewportClient && LP->ViewportClient->Viewport && LP->ViewportClient->Viewport->HasFocus();
}

bool UItemSystemUISubsystem::IsInventoryOpen() const
{
	return Screen && Screen->IsInViewport();
}

bool UItemSystemUISubsystem::HandleKeyDown(const FKey& Key, bool bIsRepeat)
{
	if (bIsRepeat) return false;
	const UItemSystemUISettings* Settings = GetDefault<UItemSystemUISettings>();

	if (IsInventoryOpen())
	{
		if (Key == Settings->ToggleKey || Key == EKeys::Escape)
		{
			CloseInventory();
			return true;
		}
		return false;
	}
	if (Key == Settings->ToggleKey && IsGameViewportFocused() && GetPlayerController())
	{
		OpenInventory();
		return true;
	}
	return false;
}

void UItemSystemUISubsystem::ToggleInventory()
{
	IsInventoryOpen() ? CloseInventory() : OpenInventory();
}

void UItemSystemUISubsystem::OpenInventory()
{
	APlayerController* PC = GetPlayerController();
	if (!PC || IsInventoryOpen()) return;

	UInventoryGridComponent* Bag = nullptr;
	UEquipmentComponent* Equipment = nullptr;
	EnsureComponents(PC, Bag, Equipment);

	const UItemSystemUISettings* Settings = GetDefault<UItemSystemUISettings>();
	TSubclassOf<UInventoryScreenWidget> ScreenClass = Settings->ScreenWidgetClass.LoadSynchronous();
	if (!ScreenClass)
	{
		ScreenClass = UInventoryScreenWidget_Code::StaticClass(); // 【临时】代码搭建的布局
	}

	Screen = CreateWidget<UInventoryScreenWidget>(PC, ScreenClass);
	if (!Screen) return;
	Screen->Setup(Bag, Equipment);
	Screen->AddToViewport(Settings->ZOrder);

	bPrevShowCursor = PC->bShowMouseCursor;
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(Screen->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
	PC->SetIgnoreLookInput(true);
}

void UItemSystemUISubsystem::CloseInventory()
{
	if (!Screen) return;
	Screen->RemoveFromParent(); // NativeDestruct 里会把手上的物品放回
	Screen = nullptr;

	if (APlayerController* PC = GetPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(bPrevShowCursor);
		PC->SetIgnoreLookInput(false);
	}
}

void UItemSystemUISubsystem::EnsureComponents(APlayerController* PC, UInventoryGridComponent*& OutBag, UEquipmentComponent*& OutEquipment)
{
	OutBag = PC->FindComponentByClass<UInventoryGridComponent>();
	OutEquipment = PC->FindComponentByClass<UEquipmentComponent>();
	bool bCreated = false;
	if (!OutBag)
	{
		OutBag = NewObject<UInventoryGridComponent>(PC, TEXT("ItemSystem_Bag"));
		OutBag->RegisterComponent();
		PC->AddInstanceComponent(OutBag);
		bCreated = true;
	}
	if (!OutEquipment)
	{
		OutEquipment = NewObject<UEquipmentComponent>(PC, TEXT("ItemSystem_Equipment"));
		OutEquipment->RegisterComponent();
		PC->AddInstanceComponent(OutEquipment);
		bCreated = true;
	}
	if (bCreated && GetDefault<UItemSystemUISettings>()->bSeedTestItems)
	{
		SeedTestItems(OutBag, OutEquipment);
	}

	// [TempRecording] 录像用：单手武器 201/2010 → 英雄换 BP_Sword/BP_Axe。删除时去掉这两行
	if (bCreated) { UTempRecordingWeaponSwap* Swap = NewObject<UTempRecordingWeaponSwap>(this); Swap->Bind(OutEquipment, PC); TempWeaponSwap = Swap; }
}

void UItemSystemUISubsystem::SeedTestItems(UInventoryGridComponent* Bag, UEquipmentComponent* Equipment)
{
	const UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(Bag);
	if (!DB) return;

	TSet<int32> EquippedTypes;
	for (int32 ID : DB->GetOrderedIDs())
	{
		const FItemBaseRow* Row = DB->Find(ID);
		if (!Row || Row->EquipType <= 0 || EquippedTypes.Contains(Row->EquipType)) continue;

		const FItemInstance Item = DB->CreateInstance(ID);
		if (!Item.IsValid()) continue;

		FItemInstance Replaced;
		if (Equipment->Equip(Item, Equipment->ResolveSlot(Item), Replaced))
		{
			EquippedTypes.Add(Row->EquipType);
		}
	}
}
