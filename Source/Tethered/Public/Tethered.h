// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/StructuredLog.h"

/** Main log category used across the project */
TETHERED_API DECLARE_LOG_CATEGORY_EXTERN(LogTethered, Log, All);

#define  CUSTOM_DEPTH_RED 250
#define  ECC_Navigation ECollisionChannel::ECC_GameTraceChannel1
#define  ECC_Interaction ECollisionChannel::ECC_GameTraceChannel2

#define  ECC_Projectile ECollisionChannel::ECC_GameTraceChannel3