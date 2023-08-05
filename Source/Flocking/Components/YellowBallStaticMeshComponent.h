#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "Flocking/Components/BallStaticMeshComponent.h"
// Generated
#include "YellowBallStaticMeshComponent.generated.h"

struct FYellowBallSettings;

/**
 * Struct to represent the collision avoidance directions.
 */
struct FDirectionHelper
{
	/** Sets default values for this struct's properties */
	FDirectionHelper()
	{
		static constexpr int32 NumViewDirections = 300;
		Directions.Init(FVector(), NumViewDirections);

		static constexpr float AngleIncrement = PI * 2.f * UE_GOLDEN_RATIO;

		TArray<int32> IndexToRemove;
		for (int32 DirectionIndex = 0; DirectionIndex < NumViewDirections; ++DirectionIndex)
		{
			const float T = static_cast<float>(DirectionIndex) / static_cast<float>(NumViewDirections);
			const float Inclination = FMath::Acos(1.f - 2.f * T);
			const float Azimuth = AngleIncrement * DirectionIndex;

			Directions[DirectionIndex].X = FMath::Cos(Inclination);
			Directions[DirectionIndex].Y = -FMath::Sin(Inclination) * FMath::Cos(Azimuth);
			Directions[DirectionIndex].Z = FMath::Sin(Inclination) * FMath::Sin(Azimuth);
		}
	}

	/** List of possible directions to check for collision avoidance */
	TArray<FVector> Directions;
};


/**
 * Static mesh component for yellow balls. It contains the associated flocking behavior.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class FLOCKING_API UYellowBallStaticMeshComponent : public UBallStaticMeshComponent
{
	GENERATED_BODY()

public:
	/// Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Deactivate() override;
	/// End UActorComponent Interface

	/** Initializes the red ball */
	void InitializeBall(FYellowBallSettings* InSettings);

	/** Add new neighbor to the list */
	FORCEINLINE void AddNeighbor(int32 NeighborIndex) { NeighborIndexes.Emplace(NeighborIndex); }

	/** Add new pursuer to the list */
	FORCEINLINE void AddPursuer(int32 PursuerIndex) { PursuerIndexes.Emplace(PursuerIndex); }

	/** Clean current neighbor list */
	FORCEINLINE void EmptyNeighbors() { NeighborIndexes.Reset(); }

	/** Clean current pursuers list */
	FORCEINLINE void EmptyPursuers() { PursuerIndexes.Reset(); }
	
private:
	/** Apply flocking forces to the movement of the yellow ball */
	void ApplySteering();

	/** Apply flee force to the movement of the yellow ball */
	void ApplyFlee();

	/** Apply collision avoidance to the movement of the yellow ball */
	void ApplyCollisionAvoidance();

	/** Calculate new velocity movement parameters */
	void CalculateNewMovementParameters(const float DeltaTime, FVector& NewLocation, FRotator& NewRotation);
	
	/** Return the acceleration to apply when adding steering force */
	FVector& SteerTowards(FVector& DesiredSteerVector) const;

private:
	/** Pointer to the settings set from the ball group actor */
	FYellowBallSettings* Settings;

	/** List of the current neighbor indexes */
	TSet<int32> NeighborIndexes;

	/** List of the current pursuer indexes */
	TSet<int32> PursuerIndexes;

	/** Current velocity of the ball (cm/s) */
	FVector Velocity{0.f, 0.f, 0.f};

	/** Acceleration applied to the ball after applying flocking and collision avoidance (cm/s2) */
	FVector Acceleration{0.f, 0.f, 0.f};
};
