#pragma once

// Minimal
#include "CoreMinimal.h"
// Parent
#include "UObject/Object.h"
// Generated
#include "BallPool.generated.h"

class UBallStaticMeshComponent;

class ActiveBallsIterator
{
public:
    ActiveBallsIterator(const int32 FirstBallIndex, const int32 LastBallIndex, TArray<bool>* InActiveFlags)
        : CurrentBallIndex(FirstBallIndex), EndBallIndex(LastBallIndex), ActiveFlags(InActiveFlags)
    {
        SkipInactiveObjects();
    }

    ActiveBallsIterator& operator++()
    {
		++CurrentBallIndex;
        SkipInactiveObjects();
        return *this;
    }

    bool operator!=(const ActiveBallsIterator& Other) const { return CurrentBallIndex != Other.CurrentBallIndex; }

    int operator*() const { return CurrentBallIndex; }

private:
    /** Current iterated ball index */
    int32 CurrentBallIndex;

    /** End ball index to iterate */
    int32 EndBallIndex;

    /** Pointer to the array of active ball flags */
    TArray<bool>* ActiveFlags;

    /** Function to automatically skip inactive balls */
    void SkipInactiveObjects()
    {
        while (CurrentBallIndex < EndBallIndex && !(*ActiveFlags)[CurrentBallIndex])
        {
            ++CurrentBallIndex;
        }
    }
};


/**
 * Object that manages a list of balls in memory.
 */
UCLASS()
class FLOCKING_API UBallPool : public UObject 
{
    GENERATED_BODY()

public:
    /** Initialize the pool with the size and the class  desired */
	void InitializePool(const int32 InitialPoolSize, UClass* InBallClass);

	/** Get a ball cast to the type desired (if there are not inactive balls, resize pool) */
	UBallStaticMeshComponent* Acquire();

    /** Release from the pool the ball desired to reuse it */
    void Release(const UBallStaticMeshComponent* Ball);

    /** Get the ball from the index selected and cast it to the type desired */
    FORCEINLINE UBallStaticMeshComponent* GetBall(const int32 BallIndex) { return Balls[BallIndex]; }

     /** Get the amount of active balls */
    FORCEINLINE int32 GetNumActive() const { return NumActive; }

    /** Begin iterator function */
    FORCEINLINE ActiveBallsIterator begin() { return ActiveBallsIterator(0, PoolSize, &ActiveFlags); }

    /** End iterator function */
    FORCEINLINE ActiveBallsIterator end() { return ActiveBallsIterator(PoolSize, PoolSize, &ActiveFlags); }

private:
    /** Dynamic resize of the pool */
    void ResizePool(int32 NewPoolSize);

private:
	/** Subclass reference to the yellow ball class */
    UPROPERTY(Transient, SkipSerialization)
    UClass* BallClass = nullptr;

    /** Contiguous array to hold balls */
    UPROPERTY(Transient, SkipSerialization)
    TArray<UBallStaticMeshComponent*> Balls;

    /** Array to track which balls are currently active */
    TArray<bool> ActiveFlags;

    /** Total amount of balls in the pool */
	int32 PoolSize = 0;

    /** Number of current active balls */
    int32 NumActive = 0;
};
