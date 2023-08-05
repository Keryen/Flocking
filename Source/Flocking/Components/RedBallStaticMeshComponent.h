#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "Flocking/Components/BallStaticMeshComponent.h"
// Generated
#include "RedBallStaticMeshComponent.generated.h"

struct FRedBallSettings;

/**
 * Static mesh component for red balls. It contains the associated flocking behavior.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class FLOCKING_API URedBallStaticMeshComponent : public UBallStaticMeshComponent
{
	GENERATED_BODY()

public:
	/// Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Deactivate() override;
	/// End UActorComponent Interface

	/** Initializes the red ball */
	void InitializeBall(FRedBallSettings* InSettings);

	/** Has any target? */
	FORCEINLINE bool HasTarget() const { return Target != nullptr; }

	/** Set a new target */
	FORCEINLINE void SetTarget(const TObjectPtr<UBallStaticMeshComponent> InTarget) { Target = InTarget; }

protected:
	/// Begin UBallStaticMeshComponent Interface
	virtual void OnRegister() override;
	/// End UBallStaticMeshComponent Interface

private:
	/** Use the current remaining energy to add more acceleration to the ball */
	void ConsumeEnergy(const float DeltaTime);

	/** Check if the ball must change its direction to follow a target */
	void CheckFollowTarget(const float DeltaTime, FRotator& MovementOrientation);

	/** Check if the ball need to bounce against a collision */
	void CheckBounce(const float DeltaTime, FRotator& MovementOrientation) const;

	/** Calculate new velocity movement parameters */
	void FollowTarget(const float DeltaTime, FRotator& RotationToTarget);

	/** Manage the target consumption */
	void ConsumeTarget();

private:
	/** Reference to the dynamic material */
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	/** Pointer of the current target to follow */
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<UBallStaticMeshComponent> Target;

	/** Pointer to the settings set from the ball group actor */
	FRedBallSettings* Settings;

	/** Current speed of the ball */
	float CurrentSpeed = 0.f;

	/** Whether the ball should follow its target */
	bool bFollowTarget = false;
	
	/** Number of yellow balls consumed */
	int32 NumYellowBallsConsumed = 0;

	/** Seconds the ball has has left to become a yellow ball */
	float RemainingTimeAlive = 0.f;

	/** Time in seconds that has past without following a target */
	float TimeWithoutFollowingTarget = 0.f;
};
