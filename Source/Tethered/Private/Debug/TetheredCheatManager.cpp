// TetheredCheatManager.cpp
#include "Debug/TetheredCheatManager.h"
#include "Components/AimAssistComponent.h"
#include "Components/CombatComponent.h"
#include "Character/TetheredPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogTetheredCheat, Log, All);

// Global combat debug state
static bool bGlobalCombatDebugEnabled = false;

UTetheredCheatManager::UTetheredCheatManager()
{
}

