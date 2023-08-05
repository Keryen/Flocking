// Class
#include "Flocking/Components/BallStaticMeshComponent.h"
// Flocking
#include "Flocking/Actors/BallGroupActor.h"

//--------------------------------------------------------------------------------------------------------------------//

UBallStaticMeshComponent::UBallStaticMeshComponent()
{
	bAutoActivate = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
}

//--------------------------------------------------------------------------------------------------------------------//

void UBallStaticMeshComponent::OnRegister()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BallGroupActor = Cast<ABallGroupActor>(GetOwner());
	check(BallGroupActor != nullptr);

	Super::OnRegister();
}

//--------------------------------------------------------------------------------------------------------------------//

void UBallStaticMeshComponent::Activate(bool bReset)
{
	Super::Activate(true);

	SetHiddenInGame(false);
}

//--------------------------------------------------------------------------------------------------------------------//

void UBallStaticMeshComponent::Deactivate()
{
	SetHiddenInGame(true);

	Super::Deactivate();
}

//--------------------------------------------------------------------------------------------------------------------//

FVector UBallStaticMeshComponent::GetLogicComponentLocation() const
{
	return GetComponentLocation() + GetForwardVector() * MeshOffset;
}

//--------------------------------------------------------------------------------------------------------------------//

FRotator UBallStaticMeshComponent::GetMovementOrientation(const float AbsRotationStep, const FRotator& EndRotation) const
{
	// Get the rotation difference
	const FRotator FacingRotator = GetComponentRotation();
	const FRotator RotationDifference(
		FMath::FindDeltaAngleDegrees(FacingRotator.Pitch, EndRotation.Pitch),
		FMath::FindDeltaAngleDegrees(FacingRotator.Yaw, EndRotation.Yaw),
		FMath::FindDeltaAngleDegrees(FacingRotator.Roll, EndRotation.Roll));

	// Apply angle cap, making sure to use the minimum value
	const FRotator RotationStep(
		FMath::Abs(RotationDifference.Pitch) <= AbsRotationStep
		? RotationDifference.Pitch
		: AbsRotationStep * FMath::Sign(RotationDifference.Pitch),
		FMath::Abs(RotationDifference.Yaw) <= AbsRotationStep
		? RotationDifference.Yaw
		: AbsRotationStep * FMath::Sign(RotationDifference.Yaw),
		FMath::Abs(RotationDifference.Roll) <= AbsRotationStep
		? RotationDifference.Roll
		: AbsRotationStep * FMath::Sign(RotationDifference.Roll));

	return FacingRotator + RotationStep;
}

//--------------------------------------------------------------------------------------------------------------------//

bool UBallStaticMeshComponent::IsHeadingForCollision(const float PredictCollisionDist, const bool bSweepSphere, FHitResult& OutHit) const
{
	const FVector ComponentLocation = GetLogicComponentLocation();

	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	QueryParams.AddIgnoredComponent(this);
	QueryParams.AddIgnoredActor(BallGroupActor);

	FCollisionResponseParams ResponseParam(ECR_Ignore);
	ResponseParam.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
	ResponseParam.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);

	bool bFoundHit;
	if (bSweepSphere)
	{
		bFoundHit = GetWorld()->SweepSingleByChannel(OutHit,
			ComponentLocation, ComponentLocation + GetForwardVector() * PredictCollisionDist, FQuat::Identity,
			ECC_WorldStatic, FCollisionShape::MakeSphere(PhysicRadius), QueryParams, ResponseParam);
	}
	else
	{
		bFoundHit = GetWorld()->LineTraceSingleByChannel(OutHit,
			ComponentLocation, ComponentLocation + GetForwardVector() * (PhysicRadius + PredictCollisionDist), 
			ECC_WorldStatic, QueryParams, ResponseParam);
	}

	return bFoundHit;
}
