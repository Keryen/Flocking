#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "GameFramework/Volume.h"
// Generated
#include "SpatialPartitioningVolume.generated.h"


/**
 * Struct to represent a voxel of the spatial partitioned volume.
 */
USTRUCT()
struct FVoxel
{
	GENERATED_BODY()

public:
	/** List of all the yellow balls */
	TSet<int32> YellowBallIndexes;

	/** List of all the red balls */
	TSet<int32> RedBallIndexes;
};


/**
 * Actor that manages the spatial partition inside the defined volume.
 */
UCLASS()
class FLOCKING_API ASpatialPartitioningVolume : public AVolume
{
	GENERATED_BODY()

public:
	/** Sets default values for this object's properties */
	ASpatialPartitioningVolume();

	/// Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif // WITH_EDITOR
	/// End UObject Interface

	/// Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	/// End AActor Interface
	
#if WITH_EDITOR
	/** Generate the voxels using the voxel size defined */
	UFUNCTION(CallInEditor, Category = "Flocking")
	void GenerateVoxels();
	
	/** Draw debug voxels */
	UFUNCTION(CallInEditor, Category = "Flocking")
	void DrawDebugVoxels() const;
#endif // WITH_EDITOR

protected:
	/// Begin AActor Interface
	virtual void BeginPlay() override;
	// End AActor Interface
	
private:
	/** Update voxel info */
	void UpdateVoxels();

	/** Update the neighbors and pursuers of the yellow ball */
	void UpdateYellowBall(const int32 YellowBallIndex, const TArray<int32>& AdjacentVoxels);

	/** Update the target of the red ball if it does not have any */
	void UpdateRedBall(const int32 RedBallIndex, const TArray<int32>& AdjacentVoxels);

	/** Get the voxel that contains the location specified */
	int32 GetVoxelPos(const FVector& Location) const;

	/** Get the voxels that are adjacent to the one specified */
	void GetAdjacentVoxelBalls(const int32 VoxelIndex, TArray<int32>& AdjacentVoxels) const;
	
protected:
	/** Size of the voxels */
	UPROPERTY(EditAnywhere, Category = "Flocking")
	int32 VoxelSize = 250;
	
	/** List of all the voxels */
	UPROPERTY()
	TArray<FVoxel> Voxels;

	/** Num of voxels in X axis */
	UPROPERTY()
	int32 NumVoxelsX = 0;

	/** Num of voxels in Y axis */
	UPROPERTY()
	int32 NumVoxelsY = 0;

	/** Num of voxels in Z axis */
	UPROPERTY()
	int32 NumVoxelsZ = 0;

	/** Offset of the volume world start location */
	UPROPERTY()
	FVector Offset;

	/** Reference to the ball group actor */
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<class ABallGroupActor> BallGroupActor;

#if WITH_EDITORONLY_DATA

	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent = nullptr;

#endif // WITH_EDITORONLY_DATA
};
