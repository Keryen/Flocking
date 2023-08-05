// Class
#include "SpatialPartitioningVolume.h"
// Flocking
#include "Flocking/Actors/BallGroupActor.h"
#include "Flocking/Components/RedBallStaticMeshComponent.h"
#include "Flocking/Components/YellowBallStaticMeshComponent.h"
// UE
#include "Components/BrushComponent.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

//--------------------------------------------------------------------------------------------------------------------//

ASpatialPartitioningVolume::ASpatialPartitioningVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetBrushComponent()->CanCharacterStepUpOn = ECB_No;
	GetBrushComponent()->SetCollisionObjectType(ECC_WorldStatic);
	GetBrushComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetBrushComponent()->SetCanEverAffectNavigation(false);
	GetBrushComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

#if WITH_EDITOR
	bIsSpatiallyLoaded = false;

	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpatialPartitioningVolumeTextureObject;
			FName ID_SpatialPartitioningVolumeGroup;
			FText NAME_SpatialPartitioningVolumeGroup;

			FConstructorStatics()
				: SpatialPartitioningVolumeTextureObject(TEXT("/Game/Textures/VolumeIcon"))
				, ID_SpatialPartitioningVolumeGroup(TEXT("SpatialPartitioningVolumeGroup"))
				, NAME_SpatialPartitioningVolumeGroup(NSLOCTEXT("SpriteCategory", "SpatialPartitioningVolumeGroup",
					  "SpatialPartitioningVolumeGroup"))
			{
			}
		};

		if (SpriteComponent != nullptr)
		{
			static FConstructorStatics ConstructorStatics;
			SpriteComponent->Sprite = ConstructorStatics.SpatialPartitioningVolumeTextureObject.Get();
			SpriteComponent->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_SpatialPartitioningVolumeGroup;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_SpatialPartitioningVolumeGroup;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->Mobility = EComponentMobility::Movable;
		}
	}
#endif // WITH_EDITOR
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::BeginPlay()
{
	Super::BeginPlay();

	BallGroupActor = Cast<ABallGroupActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ABallGroupActor::StaticClass()));
	check(BallGroupActor != nullptr);
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateVoxels();

	for (int32 VoxelIndex = 0; VoxelIndex < Voxels.Num(); ++VoxelIndex)
	{
		TArray<int32> AdjacentVoxels;
		GetAdjacentVoxelBalls(VoxelIndex, AdjacentVoxels);

		for (const int32 YellowBallIndex : Voxels[VoxelIndex].YellowBallIndexes)
		{
			UpdateYellowBall(YellowBallIndex, AdjacentVoxels);
		}
		
		for (const int32 RedBallIndex : Voxels[VoxelIndex].RedBallIndexes)
		{
			UpdateRedBall(RedBallIndex, AdjacentVoxels);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::UpdateVoxels()
{
	for (int32 VoxelIndex = 0; VoxelIndex < Voxels.Num(); ++VoxelIndex)
	{
		FVoxel& Voxel = Voxels[VoxelIndex];
		Voxel.YellowBallIndexes.Reset();
		Voxel.RedBallIndexes.Reset();
	}

	// Update yellow ball indexes
	for (const int32 ActiveBallIndex : *BallGroupActor->GetYellowBallPool())
	{
		UBallStaticMeshComponent* Ball = BallGroupActor->GetYellowBallPool()->GetBall(ActiveBallIndex);
		const int32 VoxelIndex = GetVoxelPos(Ball->GetComponentLocation());

		if (Voxels.IsValidIndex(VoxelIndex))
		{
			Voxels[VoxelIndex].YellowBallIndexes.Emplace(ActiveBallIndex);
		}
		else
		{
			Ball->Deactivate();
		}
	}

	// Update red ball indexes
	for (int32 ActiveBallIndex : *BallGroupActor->GetRedBallPool())
	{
		UBallStaticMeshComponent* Ball = BallGroupActor->GetRedBallPool()->GetBall(ActiveBallIndex);
		const int32 VoxelIndex = GetVoxelPos(Ball->GetComponentLocation());

		if (Voxels.IsValidIndex(VoxelIndex))
		{
			Voxels[VoxelIndex].RedBallIndexes.Emplace(ActiveBallIndex);
		}
		else
		{
			Ball->Deactivate();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::UpdateYellowBall(const int32 YellowBallIndex, const TArray<int32>& AdjacentVoxels)
{
	UYellowBallStaticMeshComponent* YellowBall = Cast<UYellowBallStaticMeshComponent>(BallGroupActor->GetYellowBallPool()->GetBall(YellowBallIndex));

	YellowBall->EmptyNeighbors();
	YellowBall->EmptyPursuers();

	for (const int32 AdjacentVoxel : AdjacentVoxels)
	{
		// Add adjacent yellow balls as neighbors
		for (const int32 AdjacentYellowBallIndex : Voxels[AdjacentVoxel].YellowBallIndexes)
		{
			// Ignore self index as possible neighbor
			if (AdjacentYellowBallIndex != YellowBallIndex)
			{
				YellowBall->AddNeighbor(AdjacentYellowBallIndex);
			}
		}
		
		// Add adjacent red balls as pursuers
		for (const int32 AdjacentRedBallIndex : Voxels[AdjacentVoxel].RedBallIndexes)
		{
			YellowBall->AddPursuer(AdjacentRedBallIndex);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::UpdateRedBall(const int32 RedBallIndex, const TArray<int32>& AdjacentVoxels)
{
	URedBallStaticMeshComponent* RedBall = Cast<URedBallStaticMeshComponent>(BallGroupActor->GetRedBallPool()->GetBall(RedBallIndex));

	// Do not override current target
	if (RedBall->HasTarget())
	{
		return;
	}

	for (const int32 AdjacentVoxel : AdjacentVoxels)
	{
		for (const int32 AdjacentYellowBallIndex : Voxels[AdjacentVoxel].YellowBallIndexes)
		{
			// Set the first found yellow ball as target 
			RedBall->SetTarget(BallGroupActor->GetYellowBallPool()->GetBall(AdjacentYellowBallIndex));

			return;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

int32 ASpatialPartitioningVolume::GetVoxelPos(const FVector& Location) const
{
	if (Location.X > Offset.X && Location.Y > Offset.Y && Location.Z > Offset.Z)
	{
		// Get the voxel coordinates from the ball location
		const int32 VoxelPosX = (Location.X - Offset.X) / VoxelSize;
		const int32 VoxelPosY = (Location.Y - Offset.Y) / VoxelSize;
		const int32 VoxelPosZ = (Location.Z - Offset.Z) / VoxelSize;

		//  Check if the voxel index is not over the valid ranges
		if (VoxelPosX < NumVoxelsX && VoxelPosY < NumVoxelsY && VoxelPosZ < NumVoxelsZ)
		{
			return VoxelPosX + VoxelPosY * NumVoxelsX + VoxelPosZ * NumVoxelsX * NumVoxelsY;
		}
	}

	return -1;	
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::GetAdjacentVoxelBalls(const int32 VoxelIndex, TArray<int32>& AdjacentVoxels) const
{
	// Get the voxel coordinates from the index of the array
	const int32 VoxelPosX = (VoxelIndex % (NumVoxelsX * NumVoxelsY)) % NumVoxelsX;
	const int32 VoxelPosY = (VoxelIndex % (NumVoxelsX * NumVoxelsY)) / NumVoxelsX;
	const int32 VoxelPosZ = VoxelIndex / (NumVoxelsX * NumVoxelsY);

	checkf(VoxelPosX >= 0 && VoxelPosY >= 0 && VoxelPosZ >= 0, TEXT("Invalid Voxel position"));

	// Get the coordinates of the adjacent voxels
	for (int32 AdjacentOffsetX = -1; AdjacentOffsetX <= 1; ++AdjacentOffsetX)
	{
		for (int32 AdjacentOffsetY = -1; AdjacentOffsetY <= 1; ++AdjacentOffsetY)
		{
			for (int32 AdjacentOffsetZ = -1; AdjacentOffsetZ <= 1; ++AdjacentOffsetZ)
			{
				const int32 AdjacentPosX = VoxelPosX + AdjacentOffsetX;
				const int32 AdjacentPosY = VoxelPosY + AdjacentOffsetY;
				const int32 AdjacentPosZ = VoxelPosZ + AdjacentOffsetZ;

				// If the voxel already exist, append its to check them
				if (AdjacentPosX >= 0 && AdjacentPosX < NumVoxelsX &&
					AdjacentPosY >= 0 && AdjacentPosY < NumVoxelsY &&
					AdjacentPosZ >= 0 && AdjacentPosZ < NumVoxelsZ)
				{
					const int32 AdjacentVoxelIndex =
						AdjacentPosX + AdjacentPosY * NumVoxelsX + AdjacentPosZ * NumVoxelsX * NumVoxelsY;

					check(Voxels.IsValidIndex(AdjacentVoxelIndex));
					AdjacentVoxels.Emplace(AdjacentVoxelIndex);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

#if WITH_EDITOR
void ASpatialPartitioningVolume::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName =
		PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : FName();
	const FName MemberName =
		PropertyChangedEvent.MemberProperty != nullptr ? PropertyChangedEvent.MemberProperty->GetFName() : FName();

	if (PropName == GET_MEMBER_NAME_CHECKED(ABrush, BrushBuilder)
		|| PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ASpatialPartitioningVolume, VoxelSize)
		|| MemberName == USceneComponent::GetRelativeLocationPropertyName()
		|| MemberName == USceneComponent::GetRelativeRotationPropertyName()
		|| MemberName == USceneComponent::GetRelativeScale3DPropertyName())
	{
		GenerateVoxels();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::PostEditUndo()
{
	Super::PostEditUndo();

	GenerateVoxels();
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::GenerateVoxels()
{
	const FBoxSphereBounds Bounds = GetBounds();
	const FVector GridStart = Bounds.Origin - Bounds.BoxExtent;
	const FVector GridEnd = Bounds.Origin + Bounds.BoxExtent;
	
	NumVoxelsX = FMath::CeilToInt32(FMath::Abs(GridEnd.X - GridStart.X) / VoxelSize);
	NumVoxelsY = FMath::CeilToInt32(FMath::Abs(GridEnd.Y - GridStart.Y) / VoxelSize);
	NumVoxelsZ = FMath::CeilToInt32(FMath::Abs(GridEnd.Z - GridStart.Z) / VoxelSize);

	Offset = GridStart;

	Voxels.Init(FVoxel(), NumVoxelsX * NumVoxelsY * NumVoxelsZ);

	checkf(Voxels.Num() < TNumericLimits<int32>::Max(), TEXT("The Voxel array cannot exceed the size of int32"));

	DrawDebugVoxels();
}

//--------------------------------------------------------------------------------------------------------------------//

void ASpatialPartitioningVolume::DrawDebugVoxels() const
{
	const UWorld* World = GetWorld();

	FlushPersistentDebugLines(World);

	const int32 NumVoxels = Voxels.Num();
	for (int32 VoxelIndex = 0; VoxelIndex < NumVoxels; ++VoxelIndex)
	{
		const int32 VoxelPosX = (VoxelIndex % (NumVoxelsX * NumVoxelsY)) % NumVoxelsX;
		const int32 VoxelPosY = (VoxelIndex % (NumVoxelsX * NumVoxelsY)) / NumVoxelsX;
		const int32 VoxelPosZ = VoxelIndex / (NumVoxelsX * NumVoxelsY);

		const FVector Start = FVector(
			Offset.X + VoxelPosX * VoxelSize,
			Offset.Y + VoxelPosY * VoxelSize,
			Offset.Z + VoxelPosZ * VoxelSize);

		const FVector End = FVector(
			Offset.X + VoxelPosX * VoxelSize + VoxelSize,
			Offset.Y + VoxelPosY * VoxelSize + VoxelSize,
			Offset.Z + VoxelPosZ * VoxelSize + VoxelSize);
		
		DrawDebugLine(World,FVector(End.X, End.Y, End.Z), FVector(End.X, Start.Y, End.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(End.X, Start.Y, End.Z), FVector(Start.X, Start.Y, End.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(Start.X, Start.Y, End.Z), FVector(Start.X, End.Y, End.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(Start.X, End.Y, End.Z), FVector(End.X, End.Y, End.Z),
			FColor::Yellow, false, 10.f);

		DrawDebugLine(World,FVector(End.X, End.Y, Start.Z), FVector(End.X, Start.Y, Start.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(End.X, Start.Y, Start.Z), FVector(Start.X, Start.Y, Start.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(Start.X, Start.Y, Start.Z), FVector(Start.X, End.Y, Start.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(Start.X, End.Y, Start.Z), FVector(End.X, End.Y, Start.Z),
			FColor::Yellow, false, 10.f);

		DrawDebugLine(World,FVector(End.X, End.Y, End.Z), FVector(End.X, End.Y, Start.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(End.X, Start.Y, End.Z), FVector(End.X, Start.Y, Start.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(Start.X, Start.Y, End.Z), FVector(Start.X, Start.Y, Start.Z),
			FColor::Yellow, false, 10.f);
		DrawDebugLine(World,FVector(Start.X, End.Y, End.Z), FVector(Start.X, End.Y, Start.Z),
			FColor::Yellow, false, 10.f);
	}
}
#endif //WITH_EDITOR
