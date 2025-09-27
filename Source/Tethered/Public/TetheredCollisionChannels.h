// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/**
 * Custom collision channels for the Tethered project
 * 
 * These correspond to the collision channels defined in DefaultEngine.ini
 * Use these constants instead of hardcoding ECollisionChannel values
 */
namespace TetheredCollisionChannels
{
	/** Soft collision channel - used for objects that should provide soft collision feedback */
	constexpr ECollisionChannel SoftCollision = ECC_GameTraceChannel1;
	
	/** Dash ghost channel - used during dash abilities to ignore collision with other dashing players */
	constexpr ECollisionChannel DashGhost = ECC_GameTraceChannel2;
}

/**
 * Custom collision profiles for the Tethered project
 * 
 * These correspond to the collision profiles defined in DefaultEngine.ini
 * Use these names when setting collision profiles in code
 */
namespace TetheredCollisionProfiles
{
	/** Collision profile for characters during dash ability - overlaps most objects but maintains visibility traces */
	static const FName DashGhost(TEXT("DashGhost"));
}