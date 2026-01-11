# Project Rebellion

**Dynamic Guerrilla Warfare Gamemode for Arma Reforger**

An Antistasi-inspired persistent campaign mod built on Enfusion Engine.

---

## 🎮 Features

- **Dynamic Zone Control** — Capture outposts, towns, airbases, and factories
- **Resource-Based AI** — Enemy Commander AI that doesn't cheat; uses limited resources to deploy QRFs
- **War Level Escalation** — Enemy response scales from militia trucks to helicopters and SpecOps
- **Persistent Arsenal** — Loot weapons to unlock them; deposit 25+ to make unlimited
- **Undercover System** — Blend in with civilians; avoid detection by wearing civilian clothes
- **Full Persistence** — Campaign state saves to JSON; survives server restarts

---

## 🏗️ Architecture

```
Scripts/Game/
├── AI/RBL_CommanderAI.c         # Strategic AI & QRF decisions
├── Core/
│   ├── RBL_CampaignManager.c    # Main loop, War Level, Aggression
│   ├── RBL_EconomyManager.c     # Money, HR, Virtual Arsenal
│   └── RBL_ZoneManager.c        # Zone tracking & queries
├── Systems/RBL_UndercoverSystem.c
├── Zone/RBL_CampaignZone.c      # Capture mechanics
└── Persistence/RBL_CampaignSaveData.c
```

---

## 🚀 Installation

1. Clone this repository
2. Open **Arma Reforger Tools (Workbench)**
3. **File → Add Existing Addon** → Select `addon.gproj`
4. Create a new world and place:
   - `RBL_GameMode.et`
   - `RBL_CampaignManager.et`
   - Zone prefabs (`RBL_Zone_*.et`)
5. Press **F5** to play

---

## 📋 Zone Types

| Type | Income | Description |
|------|--------|-------------|
| HQ | — | Player headquarters (lose condition if captured) |
| Airbase | 75 | Air assets, high strategic value |
| Factory | 150 | Vehicle/equipment production |
| Resource | 100 | Primary income source |
| Town | 50 | Civilian population, affects support |
| Outpost | 25 | Military garrison point |
| Seaport | 125 | Naval assets and supply routes |

---

## ⚔️ War Levels

| Level | Threshold | Enemy Assets |
|-------|-----------|--------------|
| 1 | Start | Trucks, basic rifles |
| 2 | 20 | Light infantry |
| 3 | 50 | APCs, LMGs |
| 4 | 100 | IFVs, AT weapons |
| 5 | 175 | Tanks appear |
| 6 | 275 | Transport helicopters |
| 7 | 400 | Attack helicopters |
| 8 | 550 | Heavy armor |
| 9 | 750 | Full mobilization |
| 10 | 1000 | SpecOps, scorched earth |

---

## 🛠️ Configuration

Edit `Scripts/Game/Config/RBL_Config.c` to adjust:
- Capture rates
- Aggression decay
- QRF costs and cooldowns
- Income values
- Undercover detection thresholds

---

## 📝 License

MIT License — Free to use, modify, and distribute.

---

## 🤝 Contributing

Pull requests welcome! Please follow the existing code style:
- `RBL_` prefix for all classes
- `m_` prefix for member variables
- `ERBLxxxx` for enums

