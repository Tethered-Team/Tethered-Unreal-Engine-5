// TetheredCheatManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TetheredCheatManager.generated.h"

/**
 * Custom cheat manager for Tethered project debug commands
 * Automatically disabled in shipping builds
 */
UCLASS()
class TETHERED_API UTetheredCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UTetheredCheatManager();

};