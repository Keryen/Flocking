// Class
#include "Flocking/Actors/BallGroupActor.h"
// Flocking
#include "Flocking/Components/RedBallStaticMeshComponent.h"
#include "Flocking/Components/YellowBallStaticMeshComponent.h"
// UE
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"

//--------------------------------------------------------------------------------------------------------------------//

ABallGroupActor::ABallGroupActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create a scene component
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SceneComponent->SetVisibility(false);
	SceneComponent->bHiddenInGame = false;

	// Make the scene component the root component
	RootComponent = SceneComponent;

	// Set own transform as the initial spawn point
	SpawnPoints.Emplace(GetActorTransform());

#if WITH_EDITOR
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> BallGroupTextureObject;
			FName ID_BallGroup;
			FText NAME_BallGroup;
			FConstructorStatics()
				: BallGroupTextureObject(TEXT("/Game/Textures/PacManIcon"))
				, ID_BallGroup(TEXT("BallGroup"))
				, NAME_BallGroup(NSLOCTEXT("SpriteCategory", "BallGroup", "BallGroup"))
			{
			}
		};

		if (SpriteComponent != nullptr)
		{
			static FConstructorStatics ConstructorStatics;
			SpriteComponent->Sprite = ConstructorStatics.BallGroupTextureObject.Get();
			SpriteComponent->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_BallGroup;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_BallGroup;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->Mobility = EComponentMobility::Movable;
		}
	}
#endif // WITH_EDITOR
}

//--------------------------------------------------------------------------------------------------------------------//

void ABallGroupActor::BeginPlay()
{
	Super::BeginPlay();

	YellowBallPool = NewObject<UBallPool>(this);
	YellowBallPool->InitializePool(YellowBallPoolSize, YellowBallStaticMeshComponentClass);

	RedBallPool = NewObject<UBallPool>(this);
	RedBallPool->InitializePool(RedBallPoolSize, RedBallStaticMeshComponentClass);

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

//--------------------------------------------------------------------------------------------------------------------//

void ABallGroupActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (YellowBallPool->GetNumActive() < YellowBallPoolSize)
	{
		// Increase the time since last spawn
		TimeSinceLastSpawn += DeltaSeconds;

		if (TimeSinceLastSpawn >= (1.f / YellowBallsSpawnedPerSecond))
		{
			TimeSinceLastSpawn = 0.f;

			CreateYellowBall(FTransform(
				FQuat(FMath::FRand(), FMath::FRand(), FMath::FRand(), FMath::FRand()), 
				SpawnPoints[ FMath::RandHelper(SpawnPoints.Num())].GetLocation()));
		}
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void ABallGroupActor::CreateYellowBall(const FTransform& SpawnTransform)
{
	UYellowBallStaticMeshComponent* YellowBall = Cast<UYellowBallStaticMeshComponent>(YellowBallPool->Acquire());
	YellowBall->SetupAttachment(RootComponent);
	YellowBall->RegisterComponent();

	YellowBall->SetRelativeTransform(SpawnTransform);
	YellowBall->InitializeBall(&YellowBallSettings);
}

//--------------------------------------------------------------------------------------------------------------------//

void ABallGroupActor::CreateRedBall()
{
	URedBallStaticMeshComponent* RedBall = Cast<URedBallStaticMeshComponent>(RedBallPool->Acquire());
	RedBall->SetupAttachment(RootComponent);
	RedBall->RegisterComponent();

	// Use same rotation to spawn but do it slightly in up to see the center of the ball
	FTransform SpawnTransform(PlayerController->GetControlRotation(), PlayerController->GetNavAgentLocation());
	SpawnTransform.SetLocation(SpawnTransform.GetLocation() + SpawnTransform.TransformVector(FVector::XAxisVector * 100.f));

	RedBall->SetRelativeTransform(SpawnTransform.GetRelativeTransform(GetTransform()));
	RedBall->InitializeBall(&RedBallSettings);
}