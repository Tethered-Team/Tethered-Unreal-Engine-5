// IAimable.h (UE 5.6)
#pragma once
#include "UObject/Interface.h"
#include "Aimable.generated.h"

UINTERFACE(BlueprintType)
class UAimable : public UInterface 
{ 
	GENERATED_BODY() 
};

class TETHERED_API IAimable
{
	GENERATED_BODY()
public:
	// Where to aim (head/torso). Can be a SceneComponent on the enemy.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aimable")
	USceneComponent* GetAimPointComponent() const;

	// Optional: can we target this actor right now?
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aimable")
	bool CanBeTargeted() const;
};
