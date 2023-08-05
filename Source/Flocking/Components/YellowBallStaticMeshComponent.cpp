// Class
#include "Flocking/Components/YellowBallStaticMeshComponent.h"
// Flocking
#include "Flocking/Actors/BallGroupActor.h"

//--------------------------------------------------------------------------------------------------------------------//

/** Console variables */
#if !UE_BUILD_SHIPPING
TAutoConsoleVariable CVarAdjacentDrawDebug(
	TEXT("Flocking.DrawDebugAdjacent"),
	0,
	TEXT("Draw debug adjacent balls connections. \n"),
	ECVF_Cheat);

TAutoConsoleVariable CVarSteeringDrawDebug(
	TEXT("Flocking.DrawDebugSteering"),
	0,
	TEXT("Draw debug info for steering behaviors. \n"),
	ECVF_Cheat);

TAutoConsoleVariable CVarCollisionAvoidanceDrawDebug(
	TEXT("Flocking.DrawDebugCollisionAvoidance"),
	0,
	TEXT("Draw debug info for collision avoidance. \n"),
	ECVF_Cheat);
#endif // !UE_BUILD_SHIPPING

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Acceleration = FVector::Zero();

	ApplySteering();
	ApplyFlee();
	ApplyCollisionAvoidance();

	FVector DeltaLocation;
	FRotator MovementOrientation;
	CalculateNewMovementParameters(DeltaTime, DeltaLocation, MovementOrientation);

	MoveComponent(DeltaLocation, MovementOrientation, false);
}

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::Deactivate()
{
	// Reset yellow ball properties
	NeighborIndexes.Reset();
	PursuerIndexes.Reset();
	Acceleration = FVector::Zero();

	BallGroupActor->ReleaseYellowBall(this);

	Super::Deactivate();
}

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::InitializeBall(FYellowBallSettings* InSettings)
{
	Settings = InSettings;

	// Set the movement speed based on the rand [0,1] we were given
	const float StartMovementSpeed = Settings->MovementSpeedRange.X + (Settings->MovementSpeedRange.Y - Settings->MovementSpeedRange.X) * FMath::FRand();
	Velocity = GetForwardVector() * StartMovementSpeed;

	Activate();
}

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::ApplySteering()
{
	if (NeighborIndexes.IsEmpty())
	{
		return;
	}

	const FVector ComponentLocation = GetLogicComponentLocation();

	FVector FlockHeading = FVector::ZeroVector;
	FVector FlockCentre = FVector::ZeroVector;
	FVector SeparationHeading = FVector::ZeroVector;

	// Calculate flocking parameters
	for (const int32 NeighborIndex : NeighborIndexes)
	{
		const UBallStaticMeshComponent* Neighbor = BallGroupActor->GetYellowBallPool()->GetBall(NeighborIndex);

		// Accumulate alignment
		FlockHeading += Neighbor->GetForwardVector();

		// Accumulate cohesion
		FlockCentre += Neighbor->GetLogicComponentLocation();

		// Accumulate separation
		const FVector Offset = ComponentLocation - Neighbor->GetLogicComponentLocation();
		const float DistanceSquared = Offset.SizeSquared();

		// Catch the separation radius squared to avoid the computation every call
		static const float SeparationRadiusSquared = FMath::Square(Settings->SeparationRadius);

		if (DistanceSquared < SeparationRadiusSquared)
		{
			SeparationHeading += Offset / (DistanceSquared + FLT_EPSILON);
		}

#if !UE_BUILD_SHIPPING
		if (CVarAdjacentDrawDebug.GetValueOnAnyThread() != 0)
		{
			DrawDebugLine(GetWorld(), ComponentLocation, Neighbor->GetLogicComponentLocation(),  FColor::Green);
		}
#endif // !UE_BUILD_SHIPPING
	}

	FVector OffsetToFlockCentre = FlockCentre / NeighborIndexes.Num() - ComponentLocation;

#if !UE_BUILD_SHIPPING
	if (CVarSteeringDrawDebug.GetValueOnAnyThread() != 0)
	{
		DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, ComponentLocation + FlockHeading.GetSafeNormal() * 100.f, 10.f, FColor::Blue);
		DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, ComponentLocation + OffsetToFlockCentre.GetSafeNormal() * 100.f, 10.f, FColor::Magenta);
		DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, ComponentLocation + SeparationHeading.GetSafeNormal() * 100.f, 10.f, FColor::Red);
	}
#endif // !UE_BUILD_SHIPPING

	// Apply flocking forces
	const FVector AlignmentForce = SteerTowards(FlockHeading) * Settings->AlignmentWeight;
	const FVector CohesionForce = SteerTowards(OffsetToFlockCentre) * Settings->CohesionWeight;
	const FVector SeparationForce = SteerTowards(SeparationHeading) * Settings->SeparationWeight;

	Acceleration += AlignmentForce + CohesionForce + SeparationForce;
}

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::ApplyFlee()
{
	if (PursuerIndexes.IsEmpty())
	{
		return;
	}

	const FVector ComponentLocation = GetLogicComponentLocation();

	FVector FleeHeading = FVector::ZeroVector;

	// Flee from any pursuing ball
	for (const int32 PursuerIndex : PursuerIndexes)
	{
		const UBallStaticMeshComponent* Pursuer = BallGroupActor->GetRedBallPool()->GetBall(PursuerIndex);

		const FVector Offset = ComponentLocation - Pursuer->GetLogicComponentLocation();
		const float DistanceSquared = Offset.SizeSquared();

		FleeHeading += Offset / DistanceSquared;

#if !UE_BUILD_SHIPPING
		if (CVarAdjacentDrawDebug.GetValueOnAnyThread() != 0)
		{
			DrawDebugLine(GetWorld(), ComponentLocation, Pursuer->GetLogicComponentLocation(), FColor::Black);
		}
#endif // !UE_BUILD_SHIPPING
	}

#if !UE_BUILD_SHIPPING
	if (CVarSteeringDrawDebug.GetValueOnAnyThread() != 0)
	{
		DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, ComponentLocation + FleeHeading.GetSafeNormal() * 100.f, 10.f, FColor::Orange);
	}
#endif // !UE_BUILD_SHIPPING

	// Calculate flee force
	const FVector FleeForce = SteerTowards(FleeHeading) * Settings->FleeWeight;

	Acceleration += FleeForce;
}

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::ApplyCollisionAvoidance()
{
	FHitResult OutHit;
	if (!IsHeadingForCollision(Settings->CollisionAvoidanceRadius, /* bSweepSphere = */ true, OutHit))
	{
		return;
	}

	const FVector ComponentLocation = GetLogicComponentLocation();

	FHitResult HitResult;

	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	QueryParams.AddIgnoredComponent(this);

	FCollisionResponseParams ResponseParam(ECR_Ignore);
	ResponseParam.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
	ResponseParam.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);

	// By default try avoiding the collision using the impact normal to move to the opposite dir to the impact
	FVector CollisionAvoidanceDir = (OutHit.ImpactPoint + OutHit.ImpactNormal * Settings->CollisionAvoidanceRadius * 2.f) - GetLogicComponentLocation();

	const bool bIsInsideMesh = GetWorld()->SweepSingleByChannel(HitResult, 
		ComponentLocation, ComponentLocation, FQuat::Identity, 
		ECC_WorldStatic, FCollisionShape::MakeSphere(PhysicRadius), QueryParams, ResponseParam);

	// Check if the ball is already colliding with other mesh to avoid ray-casting unnecessarily
	if (!bIsInsideMesh)
	{
		bool bFoundHit = true;

		// Catch the number of rays, it is static during all the execution
		static FDirectionHelper DirectionHelper;

		// Find the best direction to avoid the collision
		for (int32 DirectionIndex = 0; DirectionIndex < DirectionHelper.Directions.Num() && bFoundHit; ++DirectionIndex)
		{
			const FVector RayDir = GetComponentTransform().TransformVector(DirectionHelper.Directions[DirectionIndex]);

			bFoundHit = GetWorld()->SweepSingleByChannel(HitResult, 
				ComponentLocation, ComponentLocation + RayDir * Settings->CollisionAvoidanceRadius, FQuat::Identity, 
				ECC_WorldStatic, FCollisionShape::MakeSphere(PhysicRadius), QueryParams, ResponseParam);

			if (!bFoundHit)
			{
				CollisionAvoidanceDir = RayDir;
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (CVarCollisionAvoidanceDrawDebug.GetValueOnAnyThread() != 0)
	{
		DrawDebugDirectionalArrow(GetWorld(), ComponentLocation, CollisionAvoidanceDir.GetSafeNormal() * 100.f, 10.f, FColor::Yellow);
	}
#endif // !UE_BUILD_SHIPPING

	// Calculate the avoidance force
	const FVector CollisionAvoidanceForce = SteerTowards(CollisionAvoidanceDir) * Settings->CollisionAvoidanceWeight;

	Acceleration += CollisionAvoidanceForce;
}

//--------------------------------------------------------------------------------------------------------------------//

void UYellowBallStaticMeshComponent::CalculateNewMovementParameters(const float DeltaTime, FVector& NewLocation, FRotator& NewRotation)
{
	// Clamp movement speed
	Velocity = (Velocity + Acceleration * DeltaTime).GetClampedToSize(Settings->MovementSpeedRange.X, Settings->MovementSpeedRange.Y);

	// Get the final rotation of the movement
	const float AbsRotationStep = Settings->RotationSpeed * DeltaTime;
	NewRotation = GetMovementOrientation(AbsRotationStep, Velocity.Rotation());

	// Get the location to move
	NewLocation = NewRotation.Vector() * Velocity.Size() * DeltaTime;
}

//--------------------------------------------------------------------------------------------------------------------//

// TODO: Improve forces application using a lerp function to interpolate values depending on distance
// FMath::GetMappedRangeValueClamped(FVector2D(InRangeA, InRangeB), FVector2D(OutRangeA, OutRangeB), Value);
FVector& UYellowBallStaticMeshComponent::SteerTowards(FVector& DesiredSteerVector) const
{
	const FVector ForceToApply = DesiredSteerVector.GetSafeNormal() * Settings->MovementSpeedRange.Y;
	DesiredSteerVector = (ForceToApply - Velocity).GetClampedToMaxSize(Settings->MaxSteerForce);

	return DesiredSteerVector;
}
