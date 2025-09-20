// TetheredCheatManager.cpp
#include "Debug/TetheredCheatManager.h"
#include "Components/AimAssistComponent.h"
#include "Components/CombatComponent.h"
#include "Character/TetheredCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogTetheredCheat, Log, All);

// Global combat debug state
static bool bGlobalCombatDebugEnabled = false;

UTetheredCheatManager::UTetheredCheatManager()
{
}

#pragma region Aim Assist Debug Commands

void UTetheredCheatManager::ToggleAimAssistDebug()
{
	UAimAssistComponent::bGlobalDebugEnabled = !UAimAssistComponent::bGlobalDebugEnabled;
	
	const FString StatusText = UAimAssistComponent::bGlobalDebugEnabled ? TEXT("ENABLED") : TEXT("DISABLED");
	UE_LOG(LogTetheredCheat, Log, TEXT("Aim Assist Debug: %s"), *StatusText);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, 
			UAimAssistComponent::bGlobalDebugEnabled ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("Aim Assist Debug: %s"), *StatusText));
	}
}

void UTetheredCheatManager::SetAimAssistDebug(bool bEnabled)
{
	UAimAssistComponent::bGlobalDebugEnabled = bEnabled;
	
	const FString StatusText = bEnabled ? TEXT("ENABLED") : TEXT("DISABLED");
	UE_LOG(LogTetheredCheat, Log, TEXT("Aim Assist Debug: %s"), *StatusText);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, 
			bEnabled ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("Aim Assist Debug: %s"), *StatusText));
	}
}

void UTetheredCheatManager::ShowAimAssistDebugState()
{
	const FString StatusText = UAimAssistComponent::bGlobalDebugEnabled ? TEXT("ENABLED") : TEXT("DISABLED");
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
			FString::Printf(TEXT("Aim Assist Debug State: %s"), *StatusText));
	}
	
	UE_LOG(LogTetheredCheat, Log, TEXT("Aim Assist Debug State: %s"), *StatusText);
}

#pragma endregion Aim Assist Debug Commands

#pragma region Combat Debug Commands

void UTetheredCheatManager::ShowCombatDebug(bool bEnabled)
{
	bGlobalCombatDebugEnabled = bEnabled;
	
	// Also enable debug traces on the player's combat component
	if (UCombatComponent* CombatComp = GetPlayerCombatComponent())
	{
		CombatComp->bDebugShowTraces = bEnabled;
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
			FString::Printf(TEXT("Combat Debug: %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED")));
	}
	
	UE_LOG(LogTetheredCheat, Log, TEXT("Combat Debug: %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
	
	// Show additional info when enabled
	if (bEnabled)
	{
		ShowCombatStatus();
	}
}

void UTetheredCheatManager::ToggleCombatDebug()
{
	ShowCombatDebug(!bGlobalCombatDebugEnabled);
}

void UTetheredCheatManager::ShowCombatStatus()
{
	if (UCombatComponent* CombatComp = GetPlayerCombatComponent())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("Combat Status:")));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Debug Traces: %s"), CombatComp->bDebugShowTraces ? TEXT("ON") : TEXT("OFF")));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Is Attacking: %s"), CombatComp->IsAttacking() ? TEXT("YES") : TEXT("NO")));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Combo Count: %d"), CombatComp->GetComboCount()));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Is Charging: %s"), CombatComp->IsChargingAttack() ? TEXT("YES") : TEXT("NO")));
		}
		
		UE_LOG(LogTetheredCheat, Log, TEXT("Combat - Debug: %s, Attacking: %s, Combo: %d, Charging: %s"), 
			CombatComp->bDebugShowTraces ? TEXT("ON") : TEXT("OFF"),
			CombatComp->IsAttacking() ? TEXT("YES") : TEXT("NO"),
			CombatComp->GetComboCount(),
			CombatComp->IsChargingAttack() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No Combat Component found on player"));
		}
		UE_LOG(LogTetheredCheat, Warning, TEXT("No Combat Component found on player"));
	}
}

void UTetheredCheatManager::ForceAimAssistTarget(const FString& ActorName)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
			FString::Printf(TEXT("Force Aim Assist Target: %s (Not yet implemented)"), *ActorName));
	}
	
	UE_LOG(LogTetheredCheat, Warning, TEXT("ForceAimAssistTarget not yet implemented for: %s"), *ActorName);
	// TODO: Implement forced targeting functionality if needed
}

void UTetheredCheatManager::ClearForcedAimAssistTarget()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
			TEXT("Cleared Forced Aim Assist Target (Not yet implemented)"));
	}
	
	UE_LOG(LogTetheredCheat, Log, TEXT("ClearForcedAimAssistTarget called"));
	// TODO: Implement clearing forced targeting if needed
}

#pragma endregion Combat Debug Commands

#pragma region Utility Commands

void UTetheredCheatManager::ListTetheredCommands()
{
	TArray<FString> Commands = {
		TEXT("=== AIM ASSIST COMMANDS ==="),
		TEXT("ToggleAimAssistDebug - Toggle aim assist debug visualization"),
		TEXT("SetAimAssistDebug <true/false> - Set aim assist debug state"),
		TEXT("ShowAimAssistDebugState - Show current debug state"),
		TEXT("ShowAimAssistStatus - Show aim assist component status"),
		TEXT("ForceAimAssistTarget <ActorName> - Force target (WIP)"),
		TEXT("ClearForcedAimAssistTarget - Clear forced target (WIP)"),
		TEXT(""),
		TEXT("=== COMBAT COMMANDS ==="),
		TEXT("ShowCombatDebug <true/false> - Show combat debug traces"),
		TEXT("ToggleCombatDebug - Toggle combat debug traces"),
		TEXT("ShowCombatStatus - Show combat component status"),
		TEXT(""),
		TEXT("=== UTILITY COMMANDS ==="),
		TEXT("ListTetheredCommands - Show this list")
	};
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, TEXT("=== TETHERED DEBUG COMMANDS ==="));
		
		for (int32 i = 0; i < Commands.Num(); ++i)
		{
			FColor TextColor = Commands[i].StartsWith(TEXT("===")) ? FColor::Yellow : FColor::White;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, TextColor, Commands[i]);
		}
	}
	
	UE_LOG(LogTetheredCheat, Log, TEXT("=== TETHERED DEBUG COMMANDS ==="));
	for (const FString& Command : Commands)
	{
		UE_LOG(LogTetheredCheat, Log, TEXT("%s"), *Command);
	}
}

void UTetheredCheatManager::ShowAimAssistStatus()
{
	if (UAimAssistComponent* AimAssist = GetPlayerAimAssistComponent())
	{
		AActor* CurrentTarget = AimAssist->GetCurrentTarget();
		const FString TargetName = CurrentTarget ? CurrentTarget->GetName() : TEXT("None");
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("Aim Assist Status:")));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Debug: %s"), UAimAssistComponent::bGlobalDebugEnabled ? TEXT("ON") : TEXT("OFF")));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Current Target: %s"), *TargetName));
		}
		
		UE_LOG(LogTetheredCheat, Log, TEXT("Aim Assist - Debug: %s, Target: %s"), 
			UAimAssistComponent::bGlobalDebugEnabled ? TEXT("ON") : TEXT("OFF"), *TargetName);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No Aim Assist Component found on player"));
		}
		UE_LOG(LogTetheredCheat, Warning, TEXT("No Aim Assist Component found on player"));
	}
}

void UTetheredCheatManager::ShowAllDebugStatus()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::White, TEXT("=== ALL DEBUG STATUS ==="));
	}
	
	ShowAimAssistStatus();
	ShowCombatStatus();
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow,
			FString::Printf(TEXT("Global Combat Debug: %s"), bGlobalCombatDebugEnabled ? TEXT("ON") : TEXT("OFF")));
	}
}

#pragma endregion Utility Commands

#pragma region Helper Functions

UAimAssistComponent* UTetheredCheatManager::GetPlayerAimAssistComponent() const
{
	if (ATetheredCharacter* PlayerChar = GetTetheredPlayerCharacter())
	{
		return PlayerChar->GetAimAssistComponent();
	}
	return nullptr;
}

UCombatComponent* UTetheredCheatManager::GetPlayerCombatComponent() const
{
	if (ATetheredCharacter* PlayerChar = GetTetheredPlayerCharacter())
	{
		return PlayerChar->GetCombatComponent();
	}
	return nullptr;
}

ATetheredCharacter* UTetheredCheatManager::GetTetheredPlayerCharacter() const
{
	if (APlayerController* PC = GetPlayerController())
	{
		return Cast<ATetheredCharacter>(PC->GetPawn());
	}
	return nullptr;
}

bool UTetheredCheatManager::IsGlobalCombatDebugEnabled()
{
	return bGlobalCombatDebugEnabled;
}

#pragma endregion Helper Functions