// Class
#include "RedBallShooterCharacter.h"
// Flocking
#include "Flocking/Actors/BallGroupActor.h"
// UE
#include "Kismet/GameplayStatics.h"

//--------------------------------------------------------------------------------------------------------------------//

void ARedBallShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	ABallGroupActor* RedBallGroupActor = Cast<ABallGroupActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ABallGroupActor::StaticClass()));

	// Action binding
	PlayerInputComponent->BindAction(TEXT("Shoot"), IE_Pressed, RedBallGroupActor, &ABallGroupActor::CreateRedBall);
}
