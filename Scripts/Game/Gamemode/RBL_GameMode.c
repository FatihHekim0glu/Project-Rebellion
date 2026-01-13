// ============================================================================
// PROJECT REBELLION - Game Mode
// Plug-and-play: Just load this mod and play!
// ============================================================================

class RBL_GameModeClass : SCR_BaseGameModeClass
{
}

class RBL_GameMode : SCR_BaseGameMode
{
	[Attribute("1", UIWidgets.CheckBox, "Enable Rebellion systems")]
	protected bool m_bEnableRebellion;

	[Attribute("1", UIWidgets.CheckBox, "Auto-initialize zones from config")]
	protected bool m_bAutoInitialize;

	[Attribute("1", UIWidgets.CheckBox, "Show UI HUD")]
	protected bool m_bShowHUD;

	protected bool m_bSystemsInitialized;
	protected float m_fInitDelay;
	protected const float INIT_DELAY_TIME = 2.0;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!m_bEnableRebellion)
			return;

		m_bSystemsInitialized = false;
		m_fInitDelay = 0;

		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION");
		PrintFormat("[RBL] Guerrilla Warfare for Arma Reforger");
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] Waiting for world to load...");
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (!m_bEnableRebellion)
			return;

		// Delayed initialization to ensure world is ready
		if (!m_bSystemsInitialized)
		{
			m_fInitDelay += timeSlice;
			if (m_fInitDelay >= INIT_DELAY_TIME)
			{
				InitializeSystems();
				m_bSystemsInitialized = true;
			}
			return;
		}

		// Update all systems
		UpdateAllSystems(timeSlice);
	}

	protected void InitializeSystems()
	{
		PrintFormat("[RBL] Initializing Rebellion systems...");
		PrintFormat("[RBL] Network Mode: %1", RBL_NetworkUtils.IsSinglePlayer() ? "Singleplayer" : "Multiplayer");
		PrintFormat("[RBL] Authority: %1", RBL_NetworkUtils.IsServer() ? "Server" : "Client");
		
		if (m_bAutoInitialize)
		{
			RBL_AutoInitializer autoInit = RBL_AutoInitializer.GetInstance();
			autoInit.Initialize();
		}
		
		// Initialize Input System FIRST
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (inputMgr)
		{
			inputMgr.Initialize();
			PrintFormat("[RBL] Input System initialized");
		}
		
		// Initialize new systems
		RBL_CaptureManager.GetInstance();
		RBL_ShopManager.GetInstance();
		RBL_GarrisonManager.GetInstance();
		
		// Initialize Mission System (server generates missions)
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (missionMgr)
			missionMgr.Initialize();
		
		// Initialize Victory Manager
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
			victoryMgr.Initialize();
		
		// Wire campaign events to missions
		WireCampaignEventsToMissions();
		
		// Initialize NEW UI System (all clients)
		if (m_bShowHUD)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			uiMgr.Initialize();
			PrintFormat("[RBL] UI System initialized");
		}
		
		// Legacy systems (for compatibility) - Input handler uses new system
		RBL_ScreenHUD.GetInstance();
		RBL_InputHandler inputHandler = RBL_InputHandler.GetInstance();
		if (inputHandler)
			inputHandler.Initialize();
		
		// Server-only: Spawn garrisons at all enemy zones
		if (RBL_NetworkUtils.IsServer())
			GetGame().GetCallqueue().CallLater(SpawnInitialGarrisons, 5000, false);
		
		// Show welcome notification
		GetGame().GetCallqueue().CallLater(ShowWelcomeNotifications, 1000, false);
		
		PrintZoneInfo();
		PrintHelp();
		PrintNetworkInfo();
	}
	
	protected void PrintNetworkInfo()
	{
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] NETWORK STATUS");
		PrintFormat("[RBL] Mode: %1", RBL_NetworkUtils.IsSinglePlayer() ? "Singleplayer" : "Multiplayer");
		PrintFormat("[RBL] Role: %1", RBL_NetworkUtils.IsServer() ? "Server/Host" : "Client");
		PrintFormat("[RBL] Players: %1", RBL_NetworkUtils.GetPlayerCount());
		PrintFormat("[RBL] ========================================");
	}
	
	protected void SpawnInitialGarrisons()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
		{
			garMgr.SpawnAllGarrisons();
			PrintFormat("[RBL] Initial garrisons spawned");
		}
	}
	
	protected void ShowWelcomeNotifications()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
			return;
		
		uiMgr.ShowNotification("PROJECT REBELLION Active", RBL_UIColors.COLOR_ACCENT_GREEN, 4.0);
		
		GetGame().GetCallqueue().CallLater(ShowHintNotification, 2000, false);
	}
	
	protected void ShowHintNotification()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
			return;
		
		// Use dynamic keybind from input system
		string shopKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SHOP);
		uiMgr.ShowNotification(string.Format("Press [%1] to open Shop", shopKey), RBL_UIColors.COLOR_TEXT_SECONDARY, 3.0);
	}
	
	protected void WireCampaignEventsToMissions()
	{
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (!campMgr)
			return;
		
		campMgr.GetOnCampaignEvent().Insert(OnCampaignEventForMissions);
		PrintFormat("[RBL] Campaign events wired to Mission System");
	}
	
	protected void OnCampaignEventForMissions(ERBLCampaignEvent eventType, RBL_CampaignZone relatedZone)
	{
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
			return;
		
		switch (eventType)
		{
			case ERBLCampaignEvent.ZONE_CAPTURED:
				if (relatedZone)
					missionMgr.OnZoneCaptured(relatedZone.GetZoneID(), relatedZone.GetOwnerFaction());
				break;
			case ERBLCampaignEvent.ZONE_LOST:
				if (relatedZone)
					missionMgr.OnZoneCaptured(relatedZone.GetZoneID(), relatedZone.GetOwnerFaction());
				break;
			case ERBLCampaignEvent.ENEMY_KILLED:
				missionMgr.OnEnemyKilled();
				break;
		}
	}

	protected void UpdateAllSystems(float timeSlice)
	{
		// Core systems
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
		
		// Capture system
		RBL_CaptureManager captureMgr = RBL_CaptureManager.GetInstance();
		if (captureMgr)
			captureMgr.Update(timeSlice);
		
		// Garrison system
		RBL_GarrisonManager garrisonMgr = RBL_GarrisonManager.GetInstance();
		if (garrisonMgr)
			garrisonMgr.Update(timeSlice);
		
		// Undercover system
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
			undercover.Update(timeSlice);
		
		// Mission system
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (missionMgr)
			missionMgr.Update(timeSlice);
		
		// Victory system
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
			victoryMgr.Update(timeSlice);
		
		// NEW UI System
		if (m_bShowHUD)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
			{
				uiMgr.Update(timeSlice);
				uiMgr.Draw();
			}
		}
		
		// Legacy HUD (for compatibility)
		RBL_ScreenHUD hud = RBL_ScreenHUD.GetInstance();
		if (hud)
			hud.Update(timeSlice);
		
		// Input
		RBL_InputHandler input = RBL_InputHandler.GetInstance();
		if (input)
			input.Update();
	}

	protected void PrintZoneInfo()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;

		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] CAMPAIGN READY!");
		PrintFormat("[RBL] Total Zones: %1", zoneMgr.GetTotalZoneCount());
		PrintFormat("[RBL] FIA Controls: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA));
		PrintFormat("[RBL] Enemy Controls: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR));
		PrintFormat("[RBL] ========================================");
	}

	protected void PrintHelp()
	{
		// Get dynamic keybinds from input system
		string shopKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SHOP);
		string mapKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_MAP);
		string hudKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_HUD);
		string saveKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.QUICK_SAVE);
		string loadKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.QUICK_LOAD);
		string missionKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_MISSIONS);
		
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] KEYBINDS:");
		PrintFormat("[RBL]   [%1] - Open Shop", shopKey);
		PrintFormat("[RBL]   [%1] - Toggle Map", mapKey);
		PrintFormat("[RBL]   [%1] - Toggle HUD", hudKey);
		PrintFormat("[RBL]   [%1] - Toggle Missions", missionKey);
		PrintFormat("[RBL]   [%1] - Quick Save", saveKey);
		PrintFormat("[RBL]   [%1] - Quick Load", loadKey);
		PrintFormat("[RBL] CONSOLE COMMANDS:");
		PrintFormat("[RBL]   RBL_DebugCommands.PrintStatus()");
		PrintFormat("[RBL]   RBL_DebugCommands.PrintKeybinds()");
		PrintFormat("[RBL]   RBL_DebugCommands.OpenShop()");
		PrintFormat("[RBL]   RBL_DebugCommands.Buy(\"akm\")");
		PrintFormat("[RBL]   RBL_DebugCommands.AddMoney(1000)");
		PrintFormat("[RBL]   RBL_DebugCommands.ListZones()");
		PrintFormat("[RBL]   RBL_DebugCommands.CaptureZone(\"zoneid\")");
		PrintFormat("[RBL]   RBL_DebugCommands.TeleportToZone(\"zoneid\")");
		PrintFormat("[RBL]   RBL_InputTestCommands.RunInputTests()");
		PrintFormat("[RBL]   RBL_UITests.RunAllTests() - Run UI tests");
		PrintFormat("[RBL] ========================================");
	}

	void DebugPrintStatus()
	{
		RBL_DebugCommands.PrintStatus();
	}
}

// ============================================================================
// MODDED GAME MODE - Hooks into ANY existing game mode
// ============================================================================
modded class SCR_BaseGameMode
{
	protected ref RBL_GameModeAddon m_RBLAddon;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Auto-attach Rebellion to any game mode
		m_RBLAddon = new RBL_GameModeAddon();
		m_RBLAddon.OnGameModeInit(this);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (m_RBLAddon)
			m_RBLAddon.OnGameModeFrame(timeSlice);
	}
}

class RBL_GameModeAddon
{
	protected bool m_bInitialized;
	protected float m_fInitDelay;
	protected const float INIT_DELAY = 3.0;

	void RBL_GameModeAddon()
	{
		m_bInitialized = false;
		m_fInitDelay = 0;
	}

	void OnGameModeInit(SCR_BaseGameMode gameMode)
	{
		PrintFormat("[RBL] Rebellion addon attached to game mode");
	}

	void OnGameModeFrame(float timeSlice)
	{
		if (m_bInitialized)
		{
			UpdateSystems(timeSlice);
			return;
		}

		m_fInitDelay += timeSlice;
		if (m_fInitDelay >= INIT_DELAY)
		{
			Initialize();
			m_bInitialized = true;
		}
	}

	protected void Initialize()
	{
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION - ACTIVE");
		PrintFormat("[RBL] Network Mode: %1", RBL_NetworkUtils.IsSinglePlayer() ? "Singleplayer" : "Multiplayer");
		PrintFormat("[RBL] Authority: %1", RBL_NetworkUtils.IsServer() ? "Server" : "Client");
		PrintFormat("[RBL] ========================================");
		
		// Initialize Input System FIRST
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (inputMgr)
		{
			inputMgr.Initialize();
			PrintFormat("[RBL] Input System initialized");
		}
		
		// Core initialization
		RBL_AutoInitializer autoInit = RBL_AutoInitializer.GetInstance();
		autoInit.Initialize();
		
		// New systems
		RBL_CaptureManager.GetInstance();
		RBL_ShopManager.GetInstance();
		RBL_GarrisonManager.GetInstance();
		
		// Mission System (server generates missions)
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (missionMgr)
			missionMgr.Initialize();
		
		// Victory Manager
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
			victoryMgr.Initialize();
		
		// Wire campaign events
		WireCampaignEvents();
		
		// NEW UI System (all clients)
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		uiMgr.Initialize();
		
		// Legacy systems - Input handler uses new system
		RBL_ScreenHUD.GetInstance();
		RBL_InputHandler inputHandler = RBL_InputHandler.GetInstance();
		if (inputHandler)
			inputHandler.Initialize();
		
		// Server-only: Spawn garrisons after delay
		if (RBL_NetworkUtils.IsServer())
			GetGame().GetCallqueue().CallLater(SpawnGarrisons, 5000, false);
		
		// Welcome notifications
		GetGame().GetCallqueue().CallLater(ShowWelcome, 1000, false);
		
		PrintHelp();
	}
	
	protected void WireCampaignEvents()
	{
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (!campMgr)
			return;
		
		campMgr.GetOnCampaignEvent().Insert(OnCampaignEvent);
		PrintFormat("[RBL] Campaign events wired to Mission System");
	}
	
	protected void OnCampaignEvent(ERBLCampaignEvent eventType, RBL_CampaignZone relatedZone)
	{
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
			return;
		
		switch (eventType)
		{
			case ERBLCampaignEvent.ZONE_CAPTURED:
				if (relatedZone)
					missionMgr.OnZoneCaptured(relatedZone.GetZoneID(), relatedZone.GetOwnerFaction());
				break;
			case ERBLCampaignEvent.ZONE_LOST:
				if (relatedZone)
					missionMgr.OnZoneCaptured(relatedZone.GetZoneID(), relatedZone.GetOwnerFaction());
				break;
			case ERBLCampaignEvent.ENEMY_KILLED:
				missionMgr.OnEnemyKilled();
				break;
		}
	}
	
	protected void SpawnGarrisons()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
			garMgr.SpawnAllGarrisons();
	}
	
	protected void ShowWelcome()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("PROJECT REBELLION Active", RBL_UIColors.COLOR_ACCENT_GREEN, 4.0);
			GetGame().GetCallqueue().CallLater(ShowHint, 2000, false);
		}
	}
	
	protected void ShowHint()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			// Use dynamic keybind from input system
			string shopKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SHOP);
			uiMgr.ShowNotification(string.Format("Press [%1] to open Shop", shopKey), RBL_UIColors.COLOR_TEXT_SECONDARY, 3.0);
		}
	}

	protected void PrintHelp()
	{
		// Get dynamic keybinds from input system
		string shopKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SHOP);
		string mapKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_MAP);
		string hudKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_HUD);
		string saveKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.QUICK_SAVE);
		string loadKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.QUICK_LOAD);
		string missionKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_MISSIONS);
		
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] KEYBINDS:");
		PrintFormat("[RBL]   [%1] - Open Shop", shopKey);
		PrintFormat("[RBL]   [%1] - Toggle Map", mapKey);
		PrintFormat("[RBL]   [%1] - Toggle HUD", hudKey);
		PrintFormat("[RBL]   [%1] - Toggle Missions", missionKey);
		PrintFormat("[RBL]   [%1] - Quick Save", saveKey);
		PrintFormat("[RBL]   [%1] - Quick Load", loadKey);
		PrintFormat("[RBL] CONSOLE COMMANDS:");
		PrintFormat("[RBL]   RBL_DebugCommands.PrintStatus()");
		PrintFormat("[RBL]   RBL_DebugCommands.PrintKeybinds()");
		PrintFormat("[RBL]   RBL_DebugCommands.OpenShop()");
		PrintFormat("[RBL]   RBL_DebugCommands.Buy(\"akm\")");
		PrintFormat("[RBL]   RBL_DebugCommands.AddMoney(1000)");
		PrintFormat("[RBL]   RBL_DebugCommands.TeleportToZone(\"Town_LePort\")");
		PrintFormat("[RBL]   RBL_InputTestCommands.RunInputTests()");
		PrintFormat("[RBL]   RBL_UITests.RunAllTests() - Run UI tests");
		PrintFormat("[RBL] ========================================");
	}

	protected void UpdateSystems(float timeSlice)
	{
		// Core
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
		
		// Capture
		RBL_CaptureManager captureMgr = RBL_CaptureManager.GetInstance();
		if (captureMgr)
			captureMgr.Update(timeSlice);
		
		// Garrison
		RBL_GarrisonManager garrisonMgr = RBL_GarrisonManager.GetInstance();
		if (garrisonMgr)
			garrisonMgr.Update(timeSlice);
		
		// Undercover
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
			undercover.Update(timeSlice);
		
		// Mission system
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (missionMgr)
			missionMgr.Update(timeSlice);
		
		// Victory system
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
			victoryMgr.Update(timeSlice);
		
		// NEW UI System
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.Update(timeSlice);
			uiMgr.Draw();
		}
		
		// Legacy HUD
		RBL_ScreenHUD hud = RBL_ScreenHUD.GetInstance();
		if (hud)
			hud.Update(timeSlice);
		
		// Input
		RBL_InputHandler input = RBL_InputHandler.GetInstance();
		if (input)
			input.Update();
	}
}

// ============================================================================
// UI INPUT HANDLER - Keybind processing (uses RBL_InputActions constants)
// ============================================================================
class RBL_UIInputHandler
{
	protected static ref RBL_UIInputHandler s_Instance;
	
	static RBL_UIInputHandler GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_UIInputHandler();
		return s_Instance;
	}
	
	void Update()
	{
		InputManager input = GetGame().GetInputManager();
		if (!input)
			return;
		
		// Shop toggle - uses constant from RBL_InputActions
		if (input.GetActionTriggered(RBL_InputActions.TOGGLE_SHOP))
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
				uiMgr.ToggleShop();
		}
		
		// HUD toggle - uses constant from RBL_InputActions
		if (input.GetActionTriggered(RBL_InputActions.TOGGLE_HUD))
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
				uiMgr.SetVisible(!uiMgr.IsVisible());
		}
	}
}

// ============================================================================
// DEBUG COMMANDS EXTENSION - UI commands
// ============================================================================
class RBL_UIDebugCommands
{
	[ConsoleCmd("rbl_shop", "Toggle shop menu")]
	static void ToggleShop()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ToggleShop();
			PrintFormat("[RBL] Shop toggled");
		}
	}
	
	[ConsoleCmd("rbl_hud", "Toggle HUD visibility")]
	static void ToggleHUD()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.SetVisible(!uiMgr.IsVisible());
			PrintFormat("[RBL] HUD visibility: %1", uiMgr.IsVisible());
		}
	}
	
	[ConsoleCmd("rbl_notify", "Show test notification")]
	static void TestNotify()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("Test notification!", RBL_UIColors.COLOR_ACCENT_GREEN, 3.0);
			PrintFormat("[RBL] Notification sent");
		}
	}
	
	[ConsoleCmd("rbl_ui_test", "Run all UI tests")]
	static void RunUITests()
	{
		RBL_UITests.RunAllTests();
	}
}

// ============================================================================
// DEBUG COMMANDS - Mission System
// ============================================================================
class RBL_MissionDebugCommands
{
	[ConsoleCmd("rbl_missions", "List all available and active missions")]
	static void ListMissions()
	{
		RBL_MissionCommands.ListMissions();
	}
	
	[ConsoleCmd("rbl_mission_status", "Print mission system status")]
	static void PrintMissionStatus()
	{
		RBL_MissionCommands.PrintStatus();
	}
	
	[ConsoleCmd("rbl_mission_start", "Start a mission by ID")]
	static void StartMission(string missionID)
	{
		RBL_MissionCommands.StartMission(missionID);
	}
	
	[ConsoleCmd("rbl_mission_refresh", "Refresh available missions")]
	static void RefreshMissions()
	{
		RBL_MissionCommands.RefreshMissions();
	}
}
