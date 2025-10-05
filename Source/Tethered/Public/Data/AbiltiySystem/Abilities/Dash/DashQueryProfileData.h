// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DashQueryProfileData.generated.h"


/**
 * 
 */
UCLASS()
class TETHERED_API UDashQueryProfileData : public UDataAsset
{
	GENERATED_BODY()

protected:
		// Query Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (Units = "cm", ClampMin = "0"))
	float MaxDistance = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (ClampMin = "1", ClampMax = "60"))
	int32 NumSamples = 12;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query")
	bool bProjectToNav = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query")
	bool bSnapToGround = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (Units = "cm", ClampMin = "0"))
	float MaxHeightDelta = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (Units = "cm", ClampMin = "0"))
	float ClearanceBuffer = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query")
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;

};
