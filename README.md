# Project Rebellion

**Dynamic Guerrilla Warfare Campaign for Arma Reforger**

A persistent, Antistasi-inspired campaign mod built on the Enfusion Engine. Lead a resistance faction against occupying forces in a dynamic, evolving battlefield where every action has consequences.

---

## Overview

Project Rebellion transforms Arma Reforger into a full guerrilla warfare experience. Start with limited resources and equipment, capture territory, build your arsenal, recruit fighters, and escalate the conflict until you achieve victory—or face total defeat.

The mod features server-authoritative multiplayer support, allowing cooperative campaigns with friends.

---

## Features

### Core Gameplay

- **Dynamic Zone Control**: Capture and hold outposts, towns, airbases, factories, and strategic locations
- **Persistent Campaign**: Full save/load system with autosave support; campaign state survives server restarts
- **Resource-Based Economy**: Earn money and human resources from controlled territory to fund your operations
- **Progressive Arsenal**: Purchase weapons, equipment, vehicles, and recruit fighters through the in-game shop

### AI Commander System

- **Non-Cheating AI**: The enemy Commander operates on a resource pool, not infinite spawns
- **Quick Reaction Forces (QRF)**: AI deploys patrols, infantry squads, convoys, mechanized units, and helicopters based on threat assessment
- **War Level Escalation**: Enemy response scales from militia trucks to SpecOps and heavy armor as the conflict intensifies
- **Strategic Decision Making**: AI prioritizes targets based on zone value, distance, and available resources

### Undercover System

- **Civilian Disguise**: Blend in with the population by wearing civilian clothes and avoiding military equipment
- **Detection Mechanics**: Suspicion builds when carrying weapons, wearing military gear, or getting too close to enemy forces
- **Status Progression**: Move through Hidden, Suspicious, Spotted, and Compromised states

### Multiplayer Support

- **Server-Authoritative Architecture**: All critical game state is managed by the server
- **State Replication**: Economy, zones, missions, and campaign progress sync across all players
- **Cooperative Play**: Work together with other players to liberate the map

---

## Installation

### For Players

1. Subscribe to the mod on the Arma Reforger Workshop
2. Enable the mod in your mod list
3. Join a server running Project Rebellion or host your own

### For Developers

1. Clone this repository
2. Open Arma Reforger Tools (Workbench)
3. Select File, then Add Existing Addon, and choose `addon.gproj`
4. The mod auto-initializes on any game mode—no manual entity placement required
5. Press F5 to launch and test

---

## Controls

| Action | Default Key | Description |
|--------|-------------|-------------|
| Toggle Shop | J | Open/close the equipment shop |
| Toggle Settings | K | Open/close settings menu |
| Toggle HUD | H | Show/hide the main HUD |
| Toggle Missions | L | Show/hide mission log |
| Quick Save | F5 | Save game to quicksave slot |
| Quick Load | F9 | Load from quicksave slot |
| Toggle Debug HUD | F6 | Show/hide debug information |

---

## Zone Types

| Type | Base Income | Description |
|------|-------------|-------------|
| HQ | — | Headquarters; losing your HQ means defeat |
| Airbase | 75 | Air assets and high strategic value |
| Factory | 150 | Vehicle and equipment production |
| Resource | 100 | Primary income generation |
| Seaport | 125 | Naval assets and supply routes |
| Town | 50 | Civilian population; affects regional support |
| Outpost | 25 | Military garrison and patrol base |
| Roadblock | — | Minor checkpoint |

Income is modified by civilian support (50% to 150% multiplier based on local population attitude).

---

## War Level Escalation

The conflict escalates based on your cumulative score (kills + zone captures multiplied by 10).

| Level | Score Threshold | Enemy Capabilities |
|-------|-----------------|-------------------|
| 1 | Start | Trucks, basic rifles, light patrols |
| 2 | 20 | Light infantry squads |
| 3 | 50 | APCs, light machine guns |
| 4 | 100 | IFVs, anti-tank weapons |
| 5 | 175 | Tanks begin appearing |
| 6 | 275 | Transport helicopters |
| 7 | 400 | Attack helicopters |
| 8 | 550 | Heavy armor concentrations |
| 9 | 750 | Full military mobilization |
| 10 | 1000 | SpecOps teams, scorched earth tactics |

---

## Console Commands

For testing and debugging, the following console commands are available:

```
RBL_DebugCommands.PrintStatus()          Print current campaign status
RBL_DebugCommands.PrintKeybinds()        List all keybindings
RBL_DebugCommands.OpenShop()             Open the shop menu
RBL_DebugCommands.Buy("itemid")          Purchase an item directly
RBL_DebugCommands.AddMoney(amount)       Add money (debug)
RBL_DebugCommands.ListZones()            List all zones
RBL_DebugCommands.CaptureZone("zoneid")  Force capture a zone
RBL_DebugCommands.TeleportToZone("id")   Teleport to a zone

RBL_SaveCommands.Save()                  Save game
RBL_SaveCommands.Load()                  Load game
RBL_SaveCommands.SlotInfo()              Show save slot information

RBL_ShopDebugCommands.PrintShop()        Print shop catalog
RBL_ShopDebugCommands.TestPurchase("id") Test purchase an item
```

---

## Project Structure

```
Scripts/Game/
├── AI/
│   └── RBL_CommanderAI.c           Strategic AI and QRF decisions
├── Config/
│   └── RBL_Config.c                Central configuration constants
├── Core/
│   ├── RBL_AutoInitializer.c       Automatic campaign setup
│   ├── RBL_CampaignManager.c       Campaign state and events
│   ├── RBL_EconomyManager.c        Money, HR, resource management
│   ├── RBL_VictoryManager.c        Win/loss condition tracking
│   ├── RBL_ZoneConfig.c            Zone definitions
│   └── RBL_ZoneManager.c           Zone tracking and queries
├── Enums/
│   └── RBL_Enums.c                 All enumerations
├── Gamemode/
│   └── RBL_GameMode.c              Main game mode and initialization
├── Input/
│   ├── RBL_InputSystem.c           Keybind management
│   └── RBL_InputConfigValidator.c  Input validation
├── Mission/
│   ├── RBL_MissionData.c           Mission data structures
│   ├── RBL_MissionGenerator.c      Dynamic mission creation
│   └── RBL_MissionManager.c        Mission lifecycle management
├── Network/
│   ├── RBL_NetworkComponent.c      Base replication component
│   ├── RBL_NetworkManager.c        RPC handling and state sync
│   └── RBL_ReplicatedState.c       Shared network state
├── Persistence/
│   ├── RBL_AutoSaveManager.c       Autosave scheduling
│   ├── RBL_CampaignPersistence.c   Campaign state serialization
│   ├── RBL_EconomyPersistence.c    Economy state serialization
│   ├── RBL_PersistenceIntegration.c Main persistence coordinator
│   ├── RBL_PlayerPersistence.c     Player state serialization
│   ├── RBL_SaveData.c              Save data structures
│   ├── RBL_SaveFileManager.c       File I/O operations
│   ├── RBL_SaveRestorer.c          State restoration
│   ├── RBL_SaveSerializer.c        JSON serialization
│   ├── RBL_SaveSlotManager.c       Save slot management
│   └── RBL_ZonePersistence.c       Zone state serialization
├── Settings/
│   ├── RBL_Settings.c              Settings data structures
│   ├── RBL_SettingsManager.c       Settings management
│   └── RBL_SettingsPersistence.c   Settings save/load
├── Shop/
│   ├── RBL_ShopItem.c              Shop item definitions
│   └── RBL_ShopManager.c           Shop system and purchases
├── Systems/
│   ├── RBL_CaptureManager.c        Zone capture mechanics
│   ├── RBL_GarrisonManager.c       AI garrison spawning
│   ├── RBL_ItemDelivery.c          Purchase delivery system
│   └── RBL_UndercoverSystem.c      Undercover detection
├── UI/
│   ├── RBL_CaptureBarWidget.c      Capture progress display
│   ├── RBL_EndGameWidget.c         Victory/defeat screen
│   ├── RBL_HUD.c                   Legacy HUD compatibility
│   ├── RBL_InputHandler.c          UI input processing
│   ├── RBL_KeybindHintsWidget.c    Keybind hint display
│   ├── RBL_MainHUDWidget.c         Main HUD elements
│   ├── RBL_MapMarkerWidget.c       Map marker system
│   ├── RBL_MissionWidget.c         Mission display
│   ├── RBL_Notifications.c         Notification helper
│   ├── RBL_NotificationWidget.c    Notification display
│   ├── RBL_ScreenHUD.c             Debug screen HUD
│   ├── RBL_SettingsWidget.c        Settings menu UI
│   ├── RBL_ShopMenu.c              Shop menu (legacy)
│   ├── RBL_ShopMenuWidget.c        Shop menu UI
│   ├── RBL_UIBase.c                UI manager and base classes
│   ├── RBL_UIStyle.c               UI colors and styling
│   ├── RBL_UndercoverWidget.c      Undercover status display
│   └── RBL_ZoneInfoWidget.c        Zone information display
└── Zone/
    └── RBL_CampaignZone.c          Zone entity component
```

---

## Configuration

All gameplay balance values are centralized in `Scripts/Game/Config/RBL_Config.c`:

**Capture Mechanics**
- Base capture rate, unit bonus, maximum rate, decay rate

**Aggression System**
- Decay rate per minute, aggression per kill, per vehicle, per zone capture

**War Level Thresholds**
- Score requirements for each escalation level

**QRF Settings**
- Resource costs for each QRF type, cooldowns, concurrent limits

**Economy**
- Resource tick interval, income values per zone type, support multipliers

**Undercover Detection**
- Check intervals, detection radius, suspicion build/decay rates, threat values

**Starting Values**
- Initial war level, aggression, money, and human resources

---

## Technical Notes

### Network Architecture

- Server-authoritative design: clients request actions, server validates and executes
- State replication via `RplProp` and RPCs
- Capture progress broadcast at 0.5-second intervals for smooth UI updates

### Save System

- JSON-based serialization
- Multiple save slots with metadata
- Autosave on configurable interval, zone capture, and mission completion
- Quicksave/quickload support

### Code Conventions

- `RBL_` prefix for all classes
- `m_` prefix for member variables
- `s_` prefix for static variables
- `ERBL` prefix for enumerations
- Singleton pattern via `GetInstance()` for manager classes

---

## License

MIT License — Free to use, modify, and distribute.

---

## Contributing

Contributions are welcome. Please follow the existing code style and conventions. Submit pull requests with clear descriptions of changes.

---

## Credits

Inspired by Antistasi and the Arma community's long tradition of dynamic campaign missions.
