#include "Inventory/InventoryGridMath.h"

bool FInventoryGridMath::FitsInside(int32 X, int32 Y, FIntPoint Size, int32 Width, int32 Height)
{
	return X >= 0 && Y >= 0 && X + Size.X <= Width && Y + Size.Y <= Height;
}

TArray<int32> FInventoryGridMath::BuildAnchorMap(const TArray<FItemInstance>& Slots, int32 Width, int32 Height, FSizeFn SizeOf)
{
	TArray<int32> Map;
	Map.Init(-1, Width * Height);
	for (int32 Anchor = 0; Anchor < Slots.Num() && Anchor < Map.Num(); ++Anchor)
	{
		if (!Slots[Anchor].IsValid()) continue;
		const FIntPoint Size = SizeOf(Slots[Anchor]);
		const int32 X = Anchor % Width, Y = Anchor / Width;
		if (!FitsInside(X, Y, Size, Width, Height)) continue;
		for (int32 DY = 0; DY < Size.Y; ++DY)
		{
			for (int32 DX = 0; DX < Size.X; ++DX)
			{
				Map[(Y + DY) * Width + X + DX] = Anchor;
			}
		}
	}
	return Map;
}

EInventoryPlacement FInventoryGridMath::QueryPlacement(const TArray<FItemInstance>& Slots, int32 Width, int32 Height, FSizeFn SizeOf,
	int32 TargetAnchor, FIntPoint Size, int32& OutOverlapAnchor)
{
	OutOverlapAnchor = -1;
	if (Width <= 0 || TargetAnchor < 0) return EInventoryPlacement::Blocked;
	const int32 X = TargetAnchor % Width, Y = TargetAnchor / Width;
	if (!FitsInside(X, Y, Size, Width, Height)) return EInventoryPlacement::Blocked;

	const TArray<int32> Map = BuildAnchorMap(Slots, Width, Height, SizeOf);
	for (int32 DY = 0; DY < Size.Y; ++DY)
	{
		for (int32 DX = 0; DX < Size.X; ++DX)
		{
			const int32 A = Map[(Y + DY) * Width + X + DX];
			if (A < 0) continue;
			if (OutOverlapAnchor >= 0 && OutOverlapAnchor != A)
			{
				OutOverlapAnchor = -1;
				return EInventoryPlacement::Blocked; // 压住两件及以上
			}
			OutOverlapAnchor = A;
		}
	}
	return OutOverlapAnchor >= 0 ? EInventoryPlacement::Swap : EInventoryPlacement::Empty;
}

int32 FInventoryGridMath::FindFirstFit(const TArray<FItemInstance>& Slots, int32 Width, int32 Height, FSizeFn SizeOf, FIntPoint Size)
{
	for (int32 Anchor = 0; Anchor < Width * Height; ++Anchor)
	{
		int32 Overlap;
		if (QueryPlacement(Slots, Width, Height, SizeOf, Anchor, Size, Overlap) == EInventoryPlacement::Empty)
		{
			return Anchor;
		}
	}
	return -1;
}
