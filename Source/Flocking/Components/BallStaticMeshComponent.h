#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "Components/StaticMeshComponent.h"
// Generated
#include "BallStaticMeshComponent.generated.h"

/**
 * Static mesh component for balls.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class FLOCKING_API UBallStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UBallStaticMeshComponent();

	/// Begin UActorComponent Interface
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	/// End UActorComponent Interface

	/** Get the component location from the center of the ball */
	virtual FVector GetLogicComponentLocation() const;

protected:
	/// Begin UActorComponent Interface
	virtual void OnRegister() override;
	/// End UActorComponent Interface

	/** Get the orientation towards it has to rotate to */
	virtual FRotator GetMovementOrientation(const float AbsRotationStep, const FRotator& EndRotation) const;

	/** Whether the ball is going to collide with other entity */
	bool IsHeadingForCollision(const float PredictCollisionDist, const bool bSweepSphere, FHitResult& OutHit) const;

protected:
	/** Mesh offset to locate the mass center where desired */
	UPROPERTY(EditAnywhere, Category = "Flocking")
	float MeshOffset = 0.f;

	/** Reference to the group actor we belong to */
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<class ABallGroupActor> BallGroupActor;

	/** Radius of the ball mesh */
	inline static float PhysicRadius = 50.f;

	/** Time since the last spawn */
	float TimeSinceDirectionChange = 0.f;
};
