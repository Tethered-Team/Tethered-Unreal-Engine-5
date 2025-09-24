// Copyright Epic Games, Inc. All Rights Reserved.

#include "TetheredGameplayTags.h"
#include "Engine/Engine.h"

namespace TetheredGameplayTags
{

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Move, "Input.Move", "Input tag for basic movement");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Look, "Input.Look", "Input tag for camera look");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Jump, "Input.Jump", "Input tag for jumping");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Run, "Input.Run", "Input tag for running/sprinting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Dash, "Input.Dash", "Input tag for dashing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_LightAttack, "Input.LightAttack", "Input tag for light attacks");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_AlternateAttack, "Input.AlternateAttack", "Input tag for alternate attacks");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Special, "Input.Special", "Input tag for special abilities");

	// Ability Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability, "Ability", "Base ability tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Movement, "Ability.Movement", "Movement abilities");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Movement_Jump, "Ability.Movement.Jump", "Jump ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Movement_Dash, "Ability.Movement.Dash", "Dash ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Movement_Sprint, "Ability.Movement.Sprint", "Sprint ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Combat, "Ability.Combat", "Combat abilities");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Combat_LightAttack, "Ability.Combat.LightAttack", "Light attack ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Combat_AlternateAttack, "Ability.Combat.AlternateAttack", "Alternate attack ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Combat_Special, "Ability.Combat.Special", "Special attack ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Combat_ChargedAttack, "Ability.Combat.ChargedAttack", "Charged attack ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Combat_ComboAttack, "Ability.Combat.ComboAttack", "Combo attack ability");

	// Attribute Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute, "Attribute", "Base attribute tag");

	// Primary Attributes - Core character stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary, "Attribute.Primary", "Primary attributes - core character stats");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary_Health, "Attribute.Primary.Health", "Current health points");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary_MaxHealth, "Attribute.Primary.MaxHealth", "Maximum health points");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary_Stamina, "Attribute.Primary.Stamina", "Current stamina points");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary_MaxStamina, "Attribute.Primary.MaxStamina", "Maximum stamina points");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary_Mana, "Attribute.Primary.Mana", "Current mana points");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Primary_MaxMana, "Attribute.Primary.MaxMana", "Maximum mana points");

	// Secondary Attributes - Derived or calculated stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary, "Attribute.Secondary", "Secondary attributes - derived or calculated stats");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary_HealthRegen, "Attribute.Secondary.HealthRegen", "Health regeneration rate per second");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary_StaminaRegen, "Attribute.Secondary.StaminaRegen", "Stamina regeneration rate per second");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary_ManaRegen, "Attribute.Secondary.ManaRegen", "Mana regeneration rate per second");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary_MovementSpeed, "Attribute.Secondary.MovementSpeed", "Character movement speed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary_SprintMultiplier, "Attribute.Secondary.SprintMultiplier", "Movement speed multiplier when sprinting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Secondary_Armor, "Attribute.Secondary.Armor", "Armor rating for damage reduction");

	// Combat Attributes - Fighting-related stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Combat, "Attribute.Combat", "Combat attributes - fighting-related stats");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Combat_AttackDamage, "Attribute.Combat.AttackDamage", "Base attack damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Combat_AttackSpeed, "Attribute.Combat.AttackSpeed", "Attack speed multiplier");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Combat_CriticalChance, "Attribute.Combat.CriticalChance", "Critical hit chance percentage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Combat_CriticalDamage, "Attribute.Combat.CriticalDamage", "Critical hit damage multiplier");

	// Resistance Attributes - Defensive stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Resistance, "Attribute.Resistance", "Resistance attributes - defensive stats");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Resistance_Physical, "Attribute.Resistance.Physical", "Physical damage resistance percentage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Resistance_Magical, "Attribute.Resistance.Magical", "Magical damage resistance percentage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attribute_Resistance_Elemental, "Attribute.Resistance.Elemental", "Elemental damage resistance percentage");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State, "State", "Character state tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Dead, "State.Dead", "Character is dead");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Dying, "State.Dying", "Character is in the process of dying");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Invulnerable, "State.Invulnerable", "Character is invulnerable to damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Stunned, "State.Stunned", "Character is stunned");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Immobilized, "State.Immobilized", "Character cannot move");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Combat, "State.Combat", "Combat state tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Combat_Attacking, "State.Combat.Attacking", "Character is currently attacking");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Combat_Blocking, "State.Combat.Blocking", "Character is blocking");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Combat_Dodging, "State.Combat.Dodging", "Character is dodging");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement, "State.Movement", "Movement state tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Grounded, "State.Movement.Grounded", "Character is on the ground");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Airborne, "State.Movement.Airborne", "Character is in the air");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Dashing, "State.Movement.Dashing", "Character is dashing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Sprinting, "State.Movement.Sprinting", "Character is sprinting");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent, "GameplayEvent", "Gameplay event tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Damage, "GameplayEvent.Damage", "Damage event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Heal, "GameplayEvent.Heal", "Healing event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Death, "GameplayEvent.Death", "Death event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Respawn, "GameplayEvent.Respawn", "Respawn event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Combat, "GameplayEvent.Combat", "Combat event tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Combat_Hit, "GameplayEvent.Combat.Hit", "Combat hit event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Combat_Block, "GameplayEvent.Combat.Block", "Combat block event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Combat_Dodge, "GameplayEvent.Combat.Dodge", "Combat dodge event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Combat_Critical, "GameplayEvent.Combat.Critical", "Critical hit event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Movement, "GameplayEvent.Movement", "Movement event tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Movement_Landed, "GameplayEvent.Movement.Landed", "Character landed event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Movement_Jumped, "GameplayEvent.Movement.Jumped", "Character jumped event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Movement_DashStarted, "GameplayEvent.Movement.DashStarted", "Dash started event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Movement_DashEnded, "GameplayEvent.Movement.DashEnded", "Dash ended event");

	// Effect Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect, "Effect", "Gameplay effect tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Damage, "Effect.Damage", "Damage effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Healing, "Effect.Healing", "Healing effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Buff, "Effect.Buff", "Buff effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Debuff, "Effect.Debuff", "Debuff effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Damage_Physical, "Effect.Damage.Physical", "Physical damage effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Damage_Elemental, "Effect.Damage.Elemental", "Elemental damage effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Damage_Critical, "Effect.Damage.Critical", "Critical damage effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Buff_MovementSpeed, "Effect.Buff.MovementSpeed", "Movement speed buff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Buff_AttackDamage, "Effect.Buff.AttackDamage", "Attack damage buff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Buff_AttackSpeed, "Effect.Buff.AttackSpeed", "Attack speed buff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Buff_HealthRegen, "Effect.Buff.HealthRegen", "Health regeneration buff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Debuff_Slow, "Effect.Debuff.Slow", "Movement speed debuff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Debuff_Weak, "Effect.Debuff.Weak", "Attack damage debuff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Debuff_Stun, "Effect.Debuff.Stun", "Stun debuff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Debuff_Poison, "Effect.Debuff.Poison", "Poison debuff");

	// Cooldown Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown, "Cooldown", "Cooldown tags for abilities");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Dash, "Cooldown.Dash", "Dash ability cooldown");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_LightAttack, "Cooldown.LightAttack", "Light attack cooldown");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_AlternateAttack, "Cooldown.AlternateAttack", "Alternate attack cooldown");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Special, "Cooldown.Special", "Special ability cooldown");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Jump, "Cooldown.Jump", "Jump ability cooldown");

	// Cost Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cost, "Cost", "Cost tags for abilities");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cost_Health, "Cost.Health", "Health cost");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cost_Stamina, "Cost.Stamina", "Stamina cost");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cost_Mana, "Cost.Mana", "Mana cost");

	// Character Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Character, "Character", "Character type tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Character_Player, "Character.Player", "Player character");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Character_Enemy, "Character.Enemy", "Enemy character");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Character_NPC, "Character.NPC", "NPC character");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Character_Boss, "Character.Boss", "Boss character");

	// Damage Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage, "Damage", "Damage immunity and resistance tags");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage_Immunity, "Damage.Immunity", "Damage immunity");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage_Immunity_Physical, "Damage.Immunity.Physical", "Physical damage immunity");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage_Immunity_Elemental, "Damage.Immunity.Elemental", "Elemental damage immunity");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage_Resistance, "Damage.Resistance", "Damage resistance");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage_Resistance_Physical, "Damage.Resistance.Physical", "Physical damage resistance");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage_Resistance_Elemental, "Damage.Resistance.Elemental", "Elemental damage resistance");
} // namespace TetheredGameplayTags