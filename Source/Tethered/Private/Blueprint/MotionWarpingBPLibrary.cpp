// MotionWarpingBPLibrary.cpp

#include "Blueprint/MotionWarpingBPLibrary.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/Actor.h"

bool UMotionWarpingBPLibrary::AddOrUpdateWarpTarget(AActor* Actor, FName SyncPointName, const FTransform& Target)
{
    if (!Actor) return false;

    if (UMotionWarpingComponent* MW = Actor->FindComponentByClass<UMotionWarpingComponent>())
    {
        MW->AddOrUpdateWarpTargetFromTransform(SyncPointName, Target);
        return true;
    }
    return false;
}
