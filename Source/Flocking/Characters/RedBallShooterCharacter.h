#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "GameFramework/SpectatorPawn.h"
// Generated
#include "RedBallShooterCharacter.generated.h"


/**
 * Character that shoot red balls
 */
UCLASS()
class FLOCKING_API ARedBallShooterCharacter : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	/// Begin ACharacter Interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	/// End ACharacter Interface
};
