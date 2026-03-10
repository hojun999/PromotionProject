# CLAUDE.md — PromotionProject (Pcube)

This file documents the codebase structure, conventions, and development workflows for AI assistants working on this project.

---

## Project Overview

**Pcube** is a tactical RPG built with **Unreal Engine 5.7** and **C++**. It features:
- Turn-based battle system with an action queue and skill delivery mechanics
- Equipment and weapon modification system
- Exploration (World) mode with encounter triggers and cinematic cameras
- Party and inventory management across map transitions

---

## Repository Layout

```
PromotionProject/
└── Pcube/
    ├── Pcube.uproject          # Unreal project descriptor
    ├── Config/                 # Engine/game INI configuration
    │   ├── DefaultEngine.ini
    │   ├── DefaultGame.ini
    │   ├── DefaultInput.ini
    │   └── DefaultEditor.ini
    ├── Content/                # Assets and Blueprints (binary, not text)
    │   ├── Blueprints/         # 17 Blueprint classes
    │   ├── DataAssets/         # Data-driven game content (9 subdirs)
    │   ├── Levels/             # Maps: L_World, L_Battle, …
    │   ├── AnimBP/             # Animation blueprints
    │   ├── Characters/         # Skeletal mesh assets
    │   ├── Materials/
    │   ├── Widgets/            # UMG widget assets
    │   └── VFX/
    └── Source/Pcube/           # All C++ source files
        ├── Pcube.Build.cs      # Module dependencies
        ├── Pcube.Target.cs     # Standalone target
        ├── PcubeEditor.Target.cs
        └── *.h / *.cpp         # ~57 header + 57 implementation files
```

---

## Technology Stack

| Layer | Technology |
|---|---|
| Engine | Unreal Engine 5.7 |
| Language | C++ (primary), Blueprints (visual scripting) |
| Build | Unreal Build Tool (UBT) via `.Build.cs` / `.Target.cs` |
| Rendering | Ray Tracing, Substrate materials, Virtual Shadow Maps, Dynamic GI |
| Input | Enhanced Input system |
| UI | UMG (Unreal Motion Graphics) / Slate |
| AI | Unreal `AIModule` |
| Platforms | Windows (DX12/DX11), Linux (Vulkan), macOS (Metal) |

**Key UE modules** (declared in `Pcube.Build.cs`):
`Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `AIModule`, `DeveloperSettings`, `CinematicCamera`, `Slate`, `SlateCore`, `UMG`

---

## Source Code Architecture

### Class Prefixes (Unreal conventions)
| Prefix | Meaning |
|---|---|
| `A` | Actor subclass |
| `U` | UObject subclass (components, widgets, subsystems, data assets) |
| `E` | Enum |
| `F` | Struct |

### Module Naming Conventions
| Pattern | Scope |
|---|---|
| `Battle*` | Turn-based combat mode classes |
| `World*` | Exploration/overworld mode classes |
| `*DataAsset` / `*Asset` | Data-driven config objects |
| `*Subsystem` | Game-instance or world subsystems |
| `*Widget` | UMG UI widgets |
| `*HUD` | HUD classes |

---

## Key Systems

### 1. Battle System

**Entry point:** `BattleGameMode` → `BattleControlSubsystem`

**State machine (`EBattleState`):**
```
Ready → NewRound → WaitTurn → ActionInput → TargetSelection → ActionExecute → CheckCondition → Finished
```

**Key classes:**
- `ABattleBaseUnit` — base combat character (stats, skill execution, animation notify hooks)
- `ABattleAllyUnit` / `ABattleEnemyUnit` — player/enemy specializations
- `UBattleControlSubsystem` (WorldSubsystem) — state machine, target selection, result broadcast
- `ABattleTurnManager` — action order queue
- `ABattlePlayerController` — player input → action dispatch
- `ABattleHUD` — battle UI root

**Skill execution flow:**
```
Player selects skill → BattlePlayerController::HandleActionRequested()
  → BattleControlSubsystem (state: ActionExecute)
    → BattleBaseUnit::ExecuteSkill()
      → Instant: Animation notify callback
      → Projectile: ABattleProjectile spawn + collision → apply effect
```

### 2. Skill & Damage Data

`USkillDataAsset` drives all skill behaviour:
- **Target rules** (`ESkillTargetRule`): `SingleEnemy`, `AllEnemies`, `SingleAlly`, `AllAllies`, `Self`
- **Effect types** (`ESkillEffectType`): `Damage`, `Heal`, `BuffATK`, `Stun`
- **Delivery** (`ESkillDeliveryType`): `Instant` (anim-notify) or `Projectile` (spawned actor)
- Projectile count, lateral spacing, and spawn intervals are configurable per skill

Runtime modifiers live in `FSkillRuntimeSpec` / `FSkillRuntimeModifier` on `ABattleBaseUnit`.

### 3. Equipment & Weapon Mod System

- `UUnitDataAsset` — unit stats and default equipment references
- `UWeaponDataAsset` / `UArmorDataAsset` — base stats + mod slot definitions
- `UWeaponPartDataAsset` — mod item granting effects: `BonusProjectileCount`, `BonusHitCount`, `DamageMulAdd`, `DamageMulMul`
- `UItemDataAsset` — generic item (weapon part or consumable)
- `UEquipmentSubsystem` (GameInstanceSubsystem) — runtime party loadout state

Equipment slots use `SlotId`-based (not type-based) compatibility. Slot type enum: `EWeaponModSlotType`.

### 4. Inventory System

`UInventorySubsystem` (GameInstanceSubsystem):
- Max 64 item slots
- `FInventoryStack` per slot
- Broadcasts `OnInventoryChanged` delegate on mutations

### 5. World / Exploration Mode

- `AWorldPlayerController`, `AWorldHUD`, `AWorldGameMode` — overworld structure
- `AWorldBaseUnit`, `AWorldAllyUnit`, `AWorldEnemyUnit` — overworld actors
- `AWorldCCTVCameraDirector` + `AWorldCameraZoneTrigger` — area-based cinematic camera transitions
- `UWorldEncounterPlacementAsset` — defines encounter trigger positions

### 6. Encounter & Loot System

- `USpawnDataAsset` — encounter group definitions (`FUnitSpawnInfo`, `FEnemySpawnGroup`) with loot tables
- `AEncounterSpawner` — runtime spawn actor
- `ALootCorpseActor` — post-battle loot container
- `FLootDropEntry` — weighted drop entry with count ranges
- `UAllyPartyDataAsset` — default party composition (referenced in project settings)

### 7. Cross-Level State Transfer

`UBattleInfoTransferSubsystem` (GameInstanceSubsystem) carries party state, encounter info, and battle result between `L_World` and `L_Battle` maps.

---

## Data-Asset Driven Design

All game content (units, skills, weapons, armors, items, encounters) is defined as **DataAssets** in `Content/DataAssets/`. Blueprint classes in `Content/Blueprints/` extend C++ base classes to hook these assets.

**Never hardcode stat values or asset paths in C++.** Reference them through DataAssets and project settings (`UBattleProjectSettings`).

---

## Delegate Conventions

Communication between systems uses `DECLARE_DYNAMIC_MULTICAST_DELEGATE`:

| Delegate | Owner | Meaning |
|---|---|---|
| `FOnBattleStateChanged` | `BattleControlSubsystem` | State machine transition |
| `FOnTurnUnitChanged` | `BattleControlSubsystem` | Active unit updated |
| `FOnTurnOrderUpdated` | `BattleControlSubsystem` | Action queue changed |
| `FOnTargetChanged` | `BattleControlSubsystem` | Target selection changed |
| `FOnBattleFinished` | `BattleControlSubsystem` | Combat result broadcast |
| `FOnEquipmentChanged` | `EquipmentSubsystem` | Loadout mutation |
| `FOnInventoryChanged` | `InventorySubsystem` | Inventory mutation |

Prefer delegates over direct coupling between subsystems, controllers, and widgets.

---

## Configuration Files

| File | Purpose |
|---|---|
| `Config/DefaultEngine.ini` | Rendering (ray tracing, substrate, GI), default maps, nav mesh |
| `Config/DefaultGame.ini` | Asset manager, pak packaging, `DefaultAllyPartyAsset` pointer |
| `Config/DefaultInput.ini` | Enhanced Input configuration |

**Default map:** `/Game/L_World.L_World`
**Default game mode:** `/Game/GameMode/BP_WorldGameMode`

Property redirects in `DefaultEngine.ini` record all class/property renames (e.g., `BaseUnit` → `BattleBaseUnit`). **Always add a redirect when renaming a reflected property or class** to preserve saved data.

---

## Coding Conventions

1. **Language:** C++ for gameplay logic; Blueprints for content-specific overrides and UI layout.
2. **Reflection macros:** Use `UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="...")` for data assets and `UFUNCTION(BlueprintCallable)` for Blueprint-accessible methods. Always assign a `Category`.
3. **Comments:** Both Korean and English are used; domain terms often in Korean. Follow the existing style in the file you are editing.
4. **Stats:** All unit stats flow through `FUnitStats` structs defined in `UnitStatTypes.h`. Do not add raw float member variables to unit classes.
5. **Loot / drops:** Define via `FLootDropEntry` in `USpawnDataAsset`; never spawn items directly from unit death handlers.
6. **No magic numbers:** Use DataAsset fields or project settings.
7. **PCH:** Module uses `ExplicitOrSharedPCHs`; include `Pcube.h` in all `.cpp` files as the first include.

---

## Development Workflow

### Branch Convention
- Feature work committed to dated branches (e.g., `260310_2` commit style: `YYMMDD_N`)
- Claude Code sessions use `claude/<session-id>` branches

### Building
Use Unreal Editor or Unreal Build Tool directly:
```bash
# From engine installation
UnrealBuildTool Pcube Win64 Development "path/to/Pcube.uproject"
```
There are no `npm`, `make`, or `cmake` build scripts — all builds go through UBT.

### Testing
No automated test suite is configured. Testing is done through:
- In-editor Play-In-Editor (PIE) sessions on `L_Battle` and `L_World` maps
- Blueprint compile validation in Unreal Editor

### Commit Style
```
YYMMDD_N   (e.g., 260310_2)
```
Use this format for all commits to match the existing history.

---

## Maps

| Map | Purpose |
|---|---|
| `L_World` | Exploration / overworld (default startup map) |
| `L_Battle` | Turn-based combat arena |

Level transitions are triggered by `AEncounterSpawner` and managed via `UBattleInfoTransferSubsystem`.

---

## What to Avoid

- **Do not** modify `Content/` binary assets (`.uasset`, `.umap`) via text editing — use Unreal Editor.
- **Do not** skip adding `CoreRedirects` in `DefaultEngine.ini` when renaming reflected properties.
- **Do not** access subsystems via raw pointers without null-checking; use `GetWorld()->GetSubsystem<T>()`.
- **Do not** add new DataAsset types without corresponding `FPrimaryAssetType` registration in `DefaultGame.ini`.
- **Do not** create game logic in HUD or Widget classes; delegate to subsystems and controllers.
