# Tethered Custom Collision System for Dashing

## Overview
This document explains the custom collision channel and preset system implemented for player dashing in the Tethered project.

## What Was Added

### 1. Custom Collision Channels
Two new collision channels have been added to `Config/DefaultEngine.ini`:

- **SoftCollision** (`ECC_GameTraceChannel1`) - For objects that should provide soft collision feedback
- **DashGhost** (`ECC_GameTraceChannel2`) - For dash abilities to ignore collision with other dashing players

### 2. Custom Collision Profile
A new collision profile called **"DashGhost"** has been created with the following properties:
- **Collision Enabled**: Query Only
- **Object Type**: Pawn
- **Collision Responses**:
  - WorldStatic: Overlap
  - Pawn: Overlap  
  - Visibility: Block (maintains visibility traces for ground detection)
  - WorldDynamic: Overlap
  - Camera: Ignore
  - PhysicsBody: Overlap
  - Vehicle: Overlap
  - Destructible: Overlap
  - SoftCollision: Overlap
  - DashGhost: Ignore (dashing players ignore each other)

### 3. C++ Constants Header
Created `Source/Tethered/Public/TetheredCollisionChannels.h` with convenient constants:

```cpp
namespace TetheredCollisionChannels
{
    constexpr ECollisionChannel SoftCollision = ECC_GameTraceChannel1;
    constexpr ECollisionChannel DashGhost = ECC_GameTraceChannel2;
}

namespace TetheredCollisionProfiles
{
    static const FName DashGhost(TEXT("DashGhost"));
}
```

## How It Works

### During Dash Activation
1. The `UGA_DashAbility::EnableDashGhostCollision()` method saves the character's current collision settings
2. It applies the "DashGhost" collision profile to the character's capsule component
3. The character can now pass through other characters and most objects while maintaining ground detection

### During Dash Deactivation
1. The `UGA_DashAbility::DisableDashGhostCollision()` method restores the original collision settings
2. Normal collision behavior is restored

### Collision Query Enhancement
The dash query system now includes the DashGhost channel by default, ensuring proper collision detection for dash endpoints.

## Usage Examples

### In C++ Code
```cpp
// Include the collision channels header
#include "TetheredCollisionChannels.h"

// Use the collision profile
CapsuleComponent->SetCollisionProfileName(TetheredCollisionProfiles::DashGhost);

// Use collision channels in queries
TArray<TEnumAsByte<ECollisionChannel>> Channels;
Channels.Add(TetheredCollisionChannels::DashGhost);
```

### In Blueprint
- Select "DashGhost" from the Collision Presets dropdown in any collision component
- The DashGhost channel will appear in collision channel selection menus

## Benefits

1. **Clean Separation**: Dashing players can pass through each other without interference
2. **Maintained Ground Detection**: Visibility traces are preserved for proper ground snapping
3. **Configurable**: Easy to modify collision responses through the collision profile
4. **Performance**: Query-only collision reduces physics overhead during dashes
5. **Extensible**: Easy to add more custom channels and profiles as needed

## Troubleshooting

If you encounter issues:
1. Ensure the DefaultEngine.ini changes are properly applied
2. Restart the Unreal Editor after making collision profile changes
3. Check that the TetheredCollisionChannels.h file is included where needed
4. Verify collision profiles are properly restored after dash completion

## Future Enhancements

Potential improvements to consider:
- Add collision channels for different types of projectiles
- Create specialized collision profiles for different character states
- Implement collision channels for interactive objects during dashes