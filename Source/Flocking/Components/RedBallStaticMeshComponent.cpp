// Class
#include "Flocking/Components/RedBallStaticMeshComponent.h"
// Flocking
#include "Flocking/Actors/BallGroupActor.h"
#include "Flocking/Components/BallStaticMeshComponent.h"

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::OnRegister()
{
	// Set dynamic material
	DynamicMaterial = UMaterialInstanceDynamic::Create(GetMaterial(0), this);
	SetMaterial(0, DynamicMaterial);

	Super::OnRegister();
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	ConsumeEnergy(DeltaTime);
	
	FRotator MovementOrientation = GetComponentRotation();

	CheckFollowTarget(DeltaTime, MovementOrientation);
	// Override target following vector (if following a target) to bounce against a collision
	CheckBounce(DeltaTime, MovementOrientation);

	const FVector DeltaLocation = MovementOrientation.Vector() * CurrentSpeed * DeltaTime;
	MoveComponent(DeltaLocation, MovementOrientation, false);
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::Deactivate()
{
	// Reset red ball properties
	Target = nullptr;
	bFollowTarget = false;

	BallGroupActor->ReleaseRedBall(this);

	Super::Deactivate();
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::InitializeBall(FRedBallSettings* InSettings)
{
	Settings = InSettings;

	// Set initial parameter values
	CurrentSpeed = Settings->StartMovementSpeed;
	RemainingTimeAlive = Settings->MaxTimeAliveFollowingTarget;
	NumYellowBallsConsumed = 0;

	// Set initial dynamic color to red
	DynamicMaterial->SetScalarParameterValue(TEXT("Color"), 0.f);

	Activate();
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::ConsumeEnergy(const float DeltaTime)
{
	// Add acceleration if there is still energy to use
	CurrentSpeed += Settings->MovementAcceleration * DeltaTime;
	RemainingTimeAlive -= DeltaTime;

	if (RemainingTimeAlive <= 0.f)
	{
		// Transform into a yellow ball if it has consumed the max quantity of yellow balls
		if (NumYellowBallsConsumed >= Settings->MaxYellowBallConsumptions)
		{
			BallGroupActor->CreateYellowBall(GetRelativeTransform());
		}
		
		Deactivate();
	}
	// If the red ball has consumed the max quantity of yellow balls, lerp its material from red to yellow until it is not alive
	else if (NumYellowBallsConsumed >= Settings->MaxYellowBallConsumptions)
	{
		// Set the current material color
		const float ScalarColorValue = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, Settings->MaxTimeAliveFollowingTarget), FVector2D(1.f, 0.f), RemainingTimeAlive);
		DynamicMaterial->SetScalarParameterValue(TEXT("Color"), ScalarColorValue);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::CheckFollowTarget(const float DeltaTime, FRotator& MovementOrientation)
{
	// Follow target if possible
	if (bFollowTarget && HasTarget())
	{
		FollowTarget(DeltaTime, MovementOrientation);
	}
	// If following a target is in cooldown and can still consume other yellow balls, accumulate delta time
	else if (!bFollowTarget && NumYellowBallsConsumed < Settings->MaxYellowBallConsumptions)
	{
		TimeWithoutFollowingTarget += DeltaTime;

		if (TimeWithoutFollowingTarget >= Settings->FollowingTargetCooldown)
		{
			bFollowTarget = true;
			TimeWithoutFollowingTarget = 0.f;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::CheckBounce(const float DeltaTime, FRotator& MovementOrientation) const
{
	FHitResult OutHit;

	// Check if its needed to bounce
	if (IsHeadingForCollision(CurrentSpeed * DeltaTime, /* bSweepSphere = */ false, OutHit))
	{
		MovementOrientation = FMath::GetReflectionVector(GetForwardVector(), OutHit.ImpactNormal).Rotation();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::FollowTarget(const float DeltaTime, FRotator& RotationToTarget)
{
	if (!Target->IsActive())
	{
		// Stop following a target if it is not still active
		Target = nullptr;
	}
	else
	{
		static const float ConsumeTargetRadiusSquared = FMath::Square(PhysicRadius / 2.f);

		// Consume the target if it is possible
		if (FVector::DistSquared(Target->GetLogicComponentLocation(), GetLogicComponentLocation()) < ConsumeTargetRadiusSquared)
		{
			ConsumeTarget();
		}
		// Else, continue pursuing the target
		else
		{
			const FVector Direction = (Target->GetLogicComponentLocation() - GetLogicComponentLocation()).GetSafeNormal();

			// Get the final rotation of the movement
			const float AbsRotationStep = Settings->RotationSpeed * DeltaTime;
	
			const FRotator DirectionRotation = Direction.Rotation();
			RotationToTarget = GetMovementOrientation(AbsRotationStep, DirectionRotation);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void URedBallStaticMeshComponent::ConsumeTarget()
{
	Target->Deactivate();

	// Recover the initial energy
	RemainingTimeAlive = Settings->MaxTimeAliveFollowingTarget;
	
	NumYellowBallsConsumed++;

	Target = nullptr;
	bFollowTarget = false;
}
