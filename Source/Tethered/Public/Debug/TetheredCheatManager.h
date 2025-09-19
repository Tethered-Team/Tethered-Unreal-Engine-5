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

#pragma region Aim Assist Debug Commands
	
	/** Toggles aim assist debug visualization on/off */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Aim Assist")
	void ToggleAimAssistDebug();

	/** Sets aim assist debug visualization state explicitly */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Aim Assist")
	void SetAimAssistDebug(bool bEnabled);

	/** Shows current aim assist debug state */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Aim Assist")
	void ShowAimAssistDebugState();

	/** Shows current player's aim assist component status */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Aim Assist")
	void ShowAimAssistStatus();

	/** Forces a target for aim assist (debug purposes) */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Aim Assist")
	void ForceAimAssistTarget(const FString& ActorName);

	/** Clears forced aim assist target */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Aim Assist")
	void ClearForcedAimAssistTarget();

#pragma endregion Aim Assist Debug Commands

#pragma region Combat Debug Commands

	/** Shows debug information for combat traces */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Combat")
	void ShowCombatDebug(bool bEnabled = true);

	/** Toggles combat debug traces on/off */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Combat")
	void ToggleCombatDebug();

	/** Shows current combat component status */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Combat")
	void ShowCombatStatus();

#pragma endregion Combat Debug Commands

#pragma region Utility Commands

	/** Lists all available Tethered debug commands */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Utility")
	void ListTetheredCommands();

	/** Shows status of all debug systems */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tethered|Utility")
	void ShowAllDebugStatus();

#pragma endregion Utility Commands

private:
	/** Helper to get the player's aim assist component */
	class UAimAssistComponent* GetPlayerAimAssistComponent() const;

	/** Helper to get the player's combat component */
	class UCombatComponent* GetPlayerCombatComponent() const;

	/** Helper to get the player character */
	class ATetheredCharacter* GetTetheredPlayerCharacter() const;

	

public:
	/** Static helper to check if global combat debug is enabled */
	static bool IsGlobalCombatDebugEnabled();
};