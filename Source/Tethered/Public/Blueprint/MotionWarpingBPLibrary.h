// MotionWarpingBPLibrary.h
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MotionWarpingBPLibrary.generated.h"

/** Minimal glue to feed a Motion Warping sync point from Blueprints. */
UCLASS()
class TETHERED_API UMotionWarpingBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * Adds or updates a Motion Warping target on the given Actor's MotionWarpingComponent.
	 * @return true if a MotionWarpingComponent was found and updated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tethered|MotionWarping", meta = (DefaultToSelf = "Actor"))
	static bool AddOrUpdateWarpTarget(AActor* Actor, FName SyncPointName, const FTransform& Target);
};
