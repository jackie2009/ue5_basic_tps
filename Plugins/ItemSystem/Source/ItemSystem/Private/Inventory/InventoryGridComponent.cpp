#include "Inventory/InventoryGridComponent.h"

#include "Data/ItemDatabaseSubsystem.h"

UInventoryGridComponent::UInventoryGridComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UInventoryGridComponent::InitializeComponent()
{
	Super::InitializeComponent();
	EnsureSlots();
}

void UInventoryGridComponent::EnsureSlots()
{
	const int32 Count = FMath::Max(1, Width) * FMath::Max(1, Height);
	if (Slots.Num() != Count)
	{
		Slots.SetNum(Count);
	}
}

UItemDatabaseSubsystem* UInventoryGridComponent::GetDatabase() const
{
	return UItemDatabaseSubsystem::Get(this);
}

FIntPoint UInventoryGridComponent::GetItemSize(const FItemInstance& Item) const
{
	const UItemDatabaseSubsystem* DB = GetDatabase();
	return DB ? DB->GetCellSize(Item) : FIntPoint(1, 1);
}

int32 UInventoryGridComponent::FindFirstFit(FIntPoint Size) const
{
	return FInventoryGridMath::FindFirstFit(Slots, Width, Height, [this](const FItemInstance& I) { return GetItemSize(I); }, Size);
}

int32 UInventoryGridComponent::AddItem(const FItemInstance& Item)
{
	EnsureSlots();
	if (!Item.IsValid()) return -1;
	const int32 Anchor = FindFirstFit(GetItemSize(Item));
	if (Anchor >= 0)
	{
		Slots[Anchor] = Item;
		OnInventoryChanged.Broadcast();
	}
	return Anchor;
}

void UInventoryGridComponent::AddGold(int32 Amount)
{
	if (Amount == 0) return;
	Gold = FMath::Max(0, Gold + Amount);
	OnGoldChanged.Broadcast(Gold, Amount);
	OnInventoryChanged.Broadcast();
}

bool UInventoryGridComponent::FindItemAtCell(int32 Cell, FItemInstance& OutItem, int32& OutAnchor) const
{
	OutAnchor = -1;
	const TArray<int32> Map = FInventoryGridMath::BuildAnchorMap(Slots, Width, Height, [this](const FItemInstance& I) { return GetItemSize(I); });
	if (!Map.IsValidIndex(Cell) || Map[Cell] < 0) return false;
	OutAnchor = Map[Cell];
	OutItem = Slots[OutAnchor];
	return true;
}

bool UInventoryGridComponent::PickUpAtCell(int32 Cell, FItemInstance& OutItem, int32& OutAnchor)
{
	EnsureSlots();
	if (!FindItemAtCell(Cell, OutItem, OutAnchor)) return false;
	Slots[OutAnchor].Reset();
	OnInventoryChanged.Broadcast();
	return true;
}

EInventoryPlacement UInventoryGridComponent::QueryPlace(const FItemInstance& Item, int32 TargetAnchor, int32& OutOverlapAnchor) const
{
	return FInventoryGridMath::QueryPlacement(Slots, Width, Height, [this](const FItemInstance& I) { return GetItemSize(I); },
		TargetAnchor, GetItemSize(Item), OutOverlapAnchor);
}

bool UInventoryGridComponent::PlaceAt(const FItemInstance& Item, int32 TargetAnchor, FItemInstance& OutSwapped)
{
	EnsureSlots();
	OutSwapped.Reset();
	if (!Item.IsValid()) return false;

	int32 Overlap = -1;
	const EInventoryPlacement Result = QueryPlace(Item, TargetAnchor, Overlap);
	if (Result == EInventoryPlacement::Blocked) return false;
	if (Result == EInventoryPlacement::Swap)
	{
		OutSwapped = Slots[Overlap];
		Slots[Overlap].Reset();
	}
	Slots[TargetAnchor] = Item;
	OnInventoryChanged.Broadcast();
	return true;
}

TArray<FInventoryEntry> UInventoryGridComponent::GetEntries() const
{
	TArray<FInventoryEntry> Out;
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!Slots[i].IsValid()) continue;
		FInventoryEntry& E = Out.AddDefaulted_GetRef();
		E.Anchor = i;
		E.Item = Slots[i];
		E.Size = GetItemSize(Slots[i]);
	}
	return Out;
}
