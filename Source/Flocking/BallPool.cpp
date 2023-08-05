// Class
#include "Flocking/BallPool.h"
// Flocking
#include "Components/BallStaticMeshComponent.h"

//--------------------------------------------------------------------------------------------------------------------//

void UBallPool::InitializePool(const int32 InitialPoolSize, UClass* InBallClass)
{
    PoolSize = InitialPoolSize;
	BallClass = InBallClass;

	Balls.Reserve(PoolSize);
	ActiveFlags.Init(false, PoolSize);

    for (int32 i = 0; i < PoolSize; ++i)
    {
        Balls.Emplace(NewObject<UBallStaticMeshComponent>(GetOuter(), BallClass));
    }
}

//--------------------------------------------------------------------------------------------------------------------//

UBallStaticMeshComponent* UBallPool::Acquire()
{
    if (NumActive < PoolSize)
    {
        for (int32 BallIndex = 0; BallIndex < PoolSize; ++BallIndex)
        {
            if (!ActiveFlags[BallIndex])
            {
                ActiveFlags[BallIndex] = true;
                ++NumActive;
                return Balls[BallIndex];
            }
        }
    }

    // No available balls in the pool, resize the pool and try again.
    ResizePool(PoolSize * 2);
    return Acquire();
}

//--------------------------------------------------------------------------------------------------------------------//

void UBallPool::Release(const UBallStaticMeshComponent* Ball)
{
    // Ensure the released object is part of the pool
    const int32 BallIndex = Balls.IndexOfByKey(Ball);
    if (BallIndex != INDEX_NONE)
    {
        if (ActiveFlags[BallIndex])
        {
            ActiveFlags[BallIndex] = false;
            --NumActive;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------//


void UBallPool::ResizePool(const int32 NewPoolSize)
{
    if (NewPoolSize > PoolSize)
    {
        Balls.Reserve(NewPoolSize);
        ActiveFlags.Reserve(NewPoolSize);

        for (int32 i = PoolSize; i < NewPoolSize; ++i)
        {
        	Balls.Emplace(NewObject<UBallStaticMeshComponent>(GetOuter(), BallClass));
            ActiveFlags.Emplace(false);
        }

        PoolSize = NewPoolSize;
    }
}
