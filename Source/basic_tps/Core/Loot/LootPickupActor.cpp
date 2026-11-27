#include "basic_tps/Core/Loot/LootPickupActor.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Data/ItemDatabaseSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/InventoryGridComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "basic_tps/Core/Loot/LootWorldSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogLootPickup, Log, All);

void ULootLabelWidget::SetLabelData(const FString& InText, const FLinearColor& InColor, int32 InFontSize)
{
	CachedText = InText;
	CachedColor = InColor;
	CachedFontSize = InFontSize;
	ApplyLabelStyle();
}

TSharedRef<SWidget> ULootLabelWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LootLabelBg"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LootLabelText"));
		if (RootBorder && LabelText)
		{
			RootBorder->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.05f, 0.78f));
			RootBorder->SetPadding(FMargin(8.f, 3.f));
			RootBorder->SetContent(LabelText);
			WidgetTree->RootWidget = RootBorder;
		}
	}
	ApplyLabelStyle();
	return Super::RebuildWidget();
}

void ULootLabelWidget::ApplyLabelStyle()
{
	if (!LabelText) return;
	LabelText->SetText(FText::FromString(CachedText));
	LabelText->SetColorAndOpacity(FSlateColor(CachedColor));
	FSlateFontInfo Font = LabelText->GetFont();
	Font.Size = CachedFontSize;
	LabelText->SetFont(Font);
}

ALootPickupActor::ALootPickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(55.0f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 支持鼠标点击拾取
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = PickupSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.18f));
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 12.f));

	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamMesh"));
	BeamMesh->SetupAttachment(RootComponent);
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->SetCastShadow(false);
	BeamMesh->SetVisibility(false);

	LabelWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("LabelWidgetComp"));
	LabelWidgetComp->SetupAttachment(RootComponent);
	LabelWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	LabelWidgetComp->SetDrawAtDesiredSize(true);
	LabelWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 52.f));
	LabelWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BeamMesh->SetStaticMesh(CylinderMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMat.Succeeded())
	{
		VisualMesh->SetMaterial(0, BasicMat.Object);
		BeamMesh->SetMaterial(0, BasicMat.Object);
	}
}

void ALootPickupActor::BeginPlay()
{
	Super::BeginPlay();
	if (ULootWorldSubsystem* Sub = ULootWorldSubsystem::Get(this))
	{
		Sub->RegisterPickup(this);
	}
}

void ALootPickupActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULootWorldSubsystem* Sub = ULootWorldSubsystem::Get(this))
	{
		Sub->UnregisterPickup(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ALootPickupActor::InitializeLoot(const FLootDropResult& InDropResult, const FVector& InStartLocation, const FVector& InTargetLocation, float InArcHeight)
{
	DropResult = InDropResult;
	StartLoc = InStartLocation;
	TargetLoc = InTargetLocation;
	LaunchArcHeight = FMath::Max(10.0f, InArcHeight);
	ElapsedLaunchTime = 0.0f;
	bLaunching = true;
	bHasLanded = false;
	bPickedUp = false;

	SetActorLocation(StartLoc);
	SetupVisuals();
	SetupLabelWidget();
	OnLootInitialized(DropResult);
}

FString ALootPickupActor::GetDisplayLabelText() const
{
	if (DropResult.bIsGold)
	{
		return FString::Printf(TEXT("%d 金币"), DropResult.GoldAmount);
	}
	const UItemDatabaseSubsystem* DB = UItemDatabaseSubsystem::Get(this);
	if (DB && DropResult.Item.IsValid())
	{
		const FString Name = DB->GetItemDisplayName(DropResult.Item);
		if (DropResult.Item.Quality > 0)
		{
			return FString::Printf(TEXT("[%s] %s"), *UItemDatabaseSubsystem::GetQualityDisplayName(DropResult.Item.Quality), *Name);
		}
		return Name;
	}
	return TEXT("未知战利品");
}

FLinearColor ALootPickupActor::GetDisplayColor() const
{
	if (DropResult.bIsGold)
	{
		return FLinearColor(1.0f, 0.84f, 0.18f, 1.0f);
	}
	return UItemDatabaseSubsystem::GetQualityColor(DropResult.Item.Quality);
}

void ALootPickupActor::SetupVisuals()
{
	const FLinearColor Color = GetDisplayColor();

	if (VisualMesh)
	{
		if (DropResult.bIsGold)
		{
			VisualMesh->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.10f));
		}
		else
		{
			VisualMesh->SetRelativeScale3D(FVector(0.30f, 0.24f, 0.16f));
		}

		if (UMaterialInterface* BaseMat = VisualMesh->GetMaterial(0))
		{
			VisualMID = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (VisualMID)
			{
				VisualMID->SetVectorParameterValue(TEXT("Color"), Color);
				VisualMesh->SetMaterial(0, VisualMID);
			}
		}
	}

	if (BeamMesh)
	{
		const bool bShowBeam = !DropResult.bIsGold && DropResult.Item.Quality >= 1;
		BeamMesh->SetVisibility(bShowBeam);
		if (bShowBeam)
		{
			// 品质越高光柱越高、越醒目（蓝 1.2m / 稀有 2.0m / 暗金 3.2m）
			const float HeightScale = DropResult.Item.Quality >= 3 ? 3.2f : (DropResult.Item.Quality == 2 ? 2.0f : 1.2f);
			const float WidthScale = DropResult.Item.Quality >= 3 ? 0.10f : 0.06f;
			BeamMesh->SetRelativeScale3D(FVector(WidthScale, WidthScale, HeightScale));
			BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, HeightScale * 50.0f));

			if (UMaterialInterface* BaseMat = BeamMesh->GetMaterial(0))
			{
				BeamMID = UMaterialInstanceDynamic::Create(BaseMat, this);
				if (BeamMID)
				{
					BeamMID->SetVectorParameterValue(TEXT("Color"), Color * 2.5f);
					BeamMesh->SetMaterial(0, BeamMID);
				}
			}
		}
	}
}

void ALootPickupActor::SetupLabelWidget()
{
	if (!LabelWidgetComp) return;

	if (CustomLabelWidgetClass)
	{
		LabelWidgetComp->SetWidgetClass(CustomLabelWidgetClass);
		LabelWidgetComp->InitWidget();
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->IsNetMode(NM_DedicatedServer)) return;

	// 使用内置非 Abstract 的 ULootLabelWidget 渲染暗黑2 风格中文地面标签（无需手动建 WBP）
	ULootLabelWidget* LabelWidget = CreateWidget<ULootLabelWidget>(World, ULootLabelWidget::StaticClass());
	if (!LabelWidget) return;

	const int32 FontSize = (!DropResult.bIsGold && DropResult.Item.Quality >= 3) ? 15 : 13;
	LabelWidget->SetLabelData(GetDisplayLabelText(), GetDisplayColor(), FontSize);
	LabelWidgetComp->SetWidget(LabelWidget);
}

void ALootPickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bPickedUp) return;

	if (bLaunching)
	{
		ElapsedLaunchTime += DeltaTime;
		const float Alpha = LaunchDuration > KINDA_SMALL_NUMBER ? FMath::Clamp(ElapsedLaunchTime / LaunchDuration, 0.0f, 1.0f) : 1.0f;

		FVector Pos = FMath::Lerp(StartLoc, TargetLoc, Alpha);
		Pos.Z += FMath::Sin(PI * Alpha) * LaunchArcHeight;
		SetActorLocation(Pos);

		if (VisualMesh)
		{
			VisualMesh->AddLocalRotation(FRotator(0.f, 360.f * DeltaTime, 0.f));
		}

		if (Alpha >= 1.0f)
		{
			bLaunching = false;
			bHasLanded = true;
			SetActorLocation(TargetLoc);
			if (VisualMesh)
			{
				VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
			}
			OnLootLanded(DropResult);
		}
		return;
	}

	// 落地后：检查自动拾取（默认金币靠近自动拾取）
	if (bHasLanded && ((DropResult.bIsGold && bAutoPickupGold) || (!DropResult.bIsGold && bAutoPickupItems)))
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation()) <= FMath::Square(AutoPickupRadius))
			{
				TryPickupBy(PlayerPawn);
			}
		}
	}
}

void ALootPickupActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	if (bPickedUp || !bHasLanded) return;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		TryPickupBy(PlayerPawn);
	}
}

UInventoryGridComponent* ALootPickupActor::FindOrCreatePlayerBag(APawn* Picker) const
{
	if (!Picker) return nullptr;

	// 优先与 ItemSystemUI 保持同一载体：APlayerController 上的 UInventoryGridComponent
	if (APlayerController* PC = Cast<APlayerController>(Picker->GetController()))
	{
		if (UInventoryGridComponent* Bag = PC->FindComponentByClass<UInventoryGridComponent>())
		{
			return Bag;
		}
		UInventoryGridComponent* NewBag = NewObject<UInventoryGridComponent>(PC, TEXT("ItemSystem_Bag"));
		if (NewBag)
		{
			NewBag->RegisterComponent();
			PC->AddInstanceComponent(NewBag);
			return NewBag;
		}
	}

	if (UInventoryGridComponent* PawnBag = Picker->FindComponentByClass<UInventoryGridComponent>())
	{
		return PawnBag;
	}
	return nullptr;
}

bool ALootPickupActor::TryPickupBy(APawn* Picker)
{
	if (bPickedUp || !Picker) return false;

	UInventoryGridComponent* Bag = FindOrCreatePlayerBag(Picker);
	if (!Bag)
	{
		UE_LOG(LogLootPickup, Warning, TEXT("拾取失败：找不到玩家背包组件 UInventoryGridComponent"));
		return false;
	}

	if (DropResult.bIsGold)
	{
		Bag->AddGold(DropResult.GoldAmount);
		bPickedUp = true;
		OnLootPickedUp(Picker, DropResult);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor(255, 215, 45),
				FString::Printf(TEXT("拾取金币 +%d（当前: %d）"), DropResult.GoldAmount, Bag->Gold));
		}
		Destroy();
		return true;
	}

	if (!DropResult.Item.IsValid()) return false;

	const int32 Anchor = Bag->AddItem(DropResult.Item);
	if (Anchor < 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("背包空间不足，无法拾取！"));
		}
		return false;
	}

	bPickedUp = true;
	OnLootPickedUp(Picker, DropResult);
	if (GEngine)
	{
		const FColor MsgColor = GetDisplayColor().ToFColor(true);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, MsgColor,
			FString::Printf(TEXT("获得装备: %s（按 I 打开背包查看）"), *GetDisplayLabelText()));
	}
	Destroy();
	return true;
}
