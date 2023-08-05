// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "GameFramework/Actor.h"
// Flocking
#include "Flocking/BallPool.h"
// Generated
#include "BallGroupActor.generated.h"


/**
 * Struct to represent the yellow ball settings.
 */
USTRUCT()
struct FYellowBallSettings
{
	GENERATED_BODY()

	/** How quickly a yellow ball can move in cm/s (min and max values) */
	UPROPERTY(EditAnywhere, Category = "Flocking|Movement", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	FVector2D MovementSpeedRange{600.f, 750.f};

	/** Used to cap the angle so we can't perform instantaneous turns. Units: degrees/s */
	UPROPERTY(EditAnywhere, Category = "Flocking|Movement", meta = (ClampMin = "0", ClampMax = "360", Units = "Degrees"))
	float RotationSpeed = 270.f;

	/** Radius from the yellow ball to separate from other balls */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0", Units = "Centimeters"))
	float SeparationRadius = 150.f;

	/** Radius from the yellow ball to avoid entities */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0", Units = "Centimeters"))
	float CollisionAvoidanceRadius = 500.f;

	/** The weight of the alignment vector in the result */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0"))
	float AlignmentWeight = 1.f;

	/** The weight of the cohesion vector in the result */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0"))
	float CohesionWeight = 1.f;

	/** The weight of the separation vector in the result */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0"))
	float SeparationWeight = 2.f;

	/** The weight of the alignment vector in the result */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0"))
	float FleeWeight = 2.f;

	/** The weight of the collision avoidance vector vector in the result */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0"))
	float CollisionAvoidanceWeight = 10.f;

	/** Maximum steering force to apply */
	UPROPERTY(EditAnywhere, Category = "Flocking|Steering", meta = (ClampMin = "0"))
	float MaxSteerForce = 300.f;
};


/**
 * Struct to represent the red ball settings.
 */
USTRUCT()
struct FRedBallSettings
{
	GENERATED_BODY()

	/** How quickly a red ball start moving in cm/s */
	UPROPERTY(EditAnywhere, Category = "Flocking|Movement", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float StartMovementSpeed = 1500.f;

	/** Acceleration applied the red ball while it still have energy (cm/s2) */
	UPROPERTY(EditAnywhere, Category = "Flocking|Movement", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float MovementAcceleration = 50.f;

	/** Used to cap the angle so we can't perform instantaneous turns. Units: degrees/s */
	UPROPERTY(EditAnywhere, Category = "Flocking|Movement", meta = (ClampMin = "0", ClampMax = "360", Units = "Degrees"))
	float RotationSpeed = 270.f;

	/** Total amount of energy of a red ball */
	UPROPERTY(EditAnywhere, Category = "Flocking|Target", meta = (ClampMin = "0", Units = "Seconds"))
	float MaxTimeAliveFollowingTarget = 10.f;
	
	/** Amount of yellow balls to consume until die */
	UPROPERTY(EditAnywhere, Category = "Flocking|Target", meta = (ClampMin = "0"))
	int32 MaxYellowBallConsumptions = 3;

	/** Time until continue pursuing a new target */
	UPROPERTY(EditAnywhere, Category = "Flocking|Target", meta = (ClampMin = "0", Units = "Seconds"))
	float FollowingTargetCooldown = 2.f;
};


/**
 * Actor that manages a group of balls.
 */
UCLASS()
class FLOCKING_API ABallGroupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ABallGroupActor();

	/// Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	/// End AActor Interface
	
	/** Creates a new yellow ball */
	void CreateYellowBall(const FTransform& SpawnTransform);

	/** Creates a new red ball */
	void CreateRedBall();

	/** Called whenever a yellow ball becomes inactive */
	FORCEINLINE void ReleaseYellowBall(const UBallStaticMeshComponent* Ball) const { YellowBallPool->Release(Ball); };

	/** Called whenever a red ball becomes inactive */
	FORCEINLINE void ReleaseRedBall(const UBallStaticMeshComponent* Ball) const { RedBallPool->Release(Ball); };

	/** Get the pointer to the yellow ball pool */
	FORCEINLINE UBallPool* GetYellowBallPool() const { return YellowBallPool; }

	/** Get the pointer to the yellow ball pool */
	FORCEINLINE UBallPool* GetRedBallPool() const { return RedBallPool; }

protected:
	/// Begin AActor Interface
	virtual void BeginPlay() override;
	// End AActor Interface

private:
	/** Scene component used as root */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneComponent = nullptr;

	/** Subclass reference to the yellow ball class */
	UPROPERTY(EditAnywhere, Category = "Flocking|Config")
	TSubclassOf<class UYellowBallStaticMeshComponent> YellowBallStaticMeshComponentClass = nullptr;

	/** Settings to use in the generated yellow balls */
	UPROPERTY(EditAnywhere, Category = "Flocking|Config", meta=(AllowPrivateAccess="true"))
	FYellowBallSettings YellowBallSettings;

		/** Subclass reference to the red ball class */
	UPROPERTY(EditAnywhere, Category = "Flocking|Config")
	TSubclassOf<class URedBallStaticMeshComponent> RedBallStaticMeshComponentClass = nullptr;

	/** Settings to use in the generated red balls */
	UPROPERTY(EditAnywhere, Category = "Flocking|Config", meta=(AllowPrivateAccess="true"))
	FRedBallSettings RedBallSettings;
	
	/** Number of ball static mesh components to initialize the pool */
	UPROPERTY(EditAnywhere, Category = "Flocking|Spawn", meta = (ClampMin = 0, AllowPrivateAccess="true"))
	int32 YellowBallPoolSize = 1000;

	/** Units to be spawned per second */
	UPROPERTY(EditAnywhere, Category = "Flocking|Spawn", meta=(AllowPrivateAccess="true"))
	uint8 YellowBallsSpawnedPerSecond = 50;

	/** List of points where spawn */
	UPROPERTY(EditAnywhere, Category = "Flocking|Spawn", meta = (MakeEditWidget = true, AllowPrivateAccess="true"))
	TArray<FTransform> SpawnPoints;

	/** Number of ball static mesh components to initialize the pool */
	UPROPERTY(EditAnywhere, Category = "Flocking|Spawn", meta = (ClampMin = 0, AllowPrivateAccess="true"))
	int32 RedBallPoolSize = 10;

	/** Reference to the player controller */
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<AController> PlayerController = nullptr;

	/** Yellow ball pool manager */
	UPROPERTY(Transient, SkipSerialization)
	UBallPool* YellowBallPool = nullptr;

	/** Red ball pool manager */
	UPROPERTY(Transient, SkipSerialization)
	UBallPool* RedBallPool = nullptr;

	/** Time since the last spawn */
	float TimeSinceLastSpawn = 0.f;

#if WITH_EDITORONLY_DATA
	/** Sprite shown in editor to see where the actor is */
	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent = nullptr;
#endif // WITH_EDITORONLY_DATA
};
